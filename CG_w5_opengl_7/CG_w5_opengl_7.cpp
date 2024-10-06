#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h> 
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <math.h>
#include <vector>

//--- �ʿ��� ������� ����
//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ� ��
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);
GLvoid init_buffer();
void timer(int value);
void draw_shapes();
void input_rect(GLfloat* input_pos);
float random_float(float low, float high);
GLvoid Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void clamp_pos(GLfloat* input_pos);
void input_shape(char cmd, GLfloat* input_pos);
void input_tri(GLfloat* input_pos);

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VBO_dot, VBO_line, VBO_tri, VBO_rect, EBO;
std::vector<GLuint> VAO;
std::vector<GLuint> VBO;
float offset = 0;
char cmd = 0;
int shape[4] = { GL_POINTS, GL_LINES,  GL_TRIANGLES , GL_TRIANGLES };
std::vector<std::vector<unsigned int>> index = {
	{}, {}, {}, {}
};

const GLfloat colors[3][3] = { // �ﰢ�� ������ ����
{1.0, 0.0, 0.0},
{0.0, 1.0, 0.0},
{0.0, 0.0, 1.0} };

std::vector <std::vector<float >> posList = { {}, {}, {},
	{}
};


//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	srand((unsigned int)time(NULL));
	width = 800;
	height = 800;
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	//--- ���̴� �о�ͼ� ���̴� ���α׷� �����
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	shaderID = make_shaderProgram();
	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	init_buffer();
	glutMainLoop();
}

void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000, timer, 1);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'p':
	case 'l':
	case 't':
	case 'r':
		cmd = key;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{

	GLfloat input_pos[2] = { x, y };
	clamp_pos(input_pos);
	if (state == GLUT_DOWN) {
		input_shape(cmd, input_pos);
		
		glutPostRedisplay();
	}
	else {
	}

}

void clamp_pos(GLfloat* input_pos) {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int viewport_width = viewport[2];
	int viewport_height = viewport[3];
	input_pos[0] = (input_pos[0] / viewport_width) * 2 - 1.0f;
	input_pos[1] = -1 * ((input_pos[1] / viewport_height) * 2 - 1.0f);
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders() {
	GLchar* fragmentSource;
	fragmentSource = filetobuf("fragment.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram() {
	GLint result;
	GLchar errorLog[512];
	shaderID = glCreateProgram(); 
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID);
	return shaderID;
}

GLvoid drawScene() {
	GLfloat rColor, gColor, bColor;
	rColor = bColor = 0.0;
	gColor = 0.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderID);

	draw_shapes();
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void init_buffer() {
	VAO.assign(4, NULL);
	VBO.assign(4, NULL);

	for (int i = 0; i < 4; i++) {
		glGenVertexArrays(1, &VAO[i]);
		glGenBuffers(1, &VBO[i]);

	}
	glGenBuffers(1, &EBO);
}

void draw_shapes() {
	for (int i = 0; i < 4; i++) {
		if (posList.empty()) continue;
		glBindVertexArray(VAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, posList[i].size() * sizeof(float), posList[i].data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index[i].size() * sizeof(float), index[i].data(), GL_STATIC_DRAW);
		std::cout << posList.size() << std::endl;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glDrawElements(shape[i], index[i].size(), GL_UNSIGNED_INT, 0);
	}
}

void input_shape(char cmd, GLfloat* input_pos) {
	switch (cmd) {
	case 'p':

		break;
	case 'l':
		break;

	case 't':
		input_tri(input_pos);
		break;

	case 'r':
		input_rect(input_pos);
		break;
	}
}

void input_tri(GLfloat* input_pos) {
	float lx[3] = { 0, -0.5, 0.5};
	float ly[3] = { 1, -0.5, -0.5};

	float radius = 0.5f;

	float r = random_float(0.3, 1);
	float g = random_float(0.3, 1);
	float b = random_float(0.3, 1);

	int lastindex = index[2].size() / 3 * 3;

	for (int i = 0; i < 3; i++) {
		posList[2].push_back(input_pos[0] + radius / 2 * lx[i]);
		posList[2].push_back(input_pos[1] + radius / 2 * ly[i]);
		posList[2].push_back(0.0f);
		posList[2].push_back(r);
		posList[2].push_back(g);
		posList[2].push_back(b);
	}

	index[2].push_back(lastindex);
	index[2].push_back(lastindex + 1);
	index[2].push_back(lastindex + 2);

}

void input_rect(GLfloat* input_pos) {
	float lx[4] = { -1, -1, 1, 1 };
	float ly[4] = { -1, 1, -1, 1 };

	float width = random_float(0.3, 0.5);
	float height = random_float(0.3, 0.5);

	float r = random_float(0.3, 1);
	float g = random_float(0.3, 1);
	float b = random_float(0.3, 1);

	int lastindex = index[3].size() / 6 * 4;

	for (int i = 0; i < 4; i++) {
		posList[3].push_back(input_pos[0] + width / 2 * lx[i]);
		posList[3].push_back(input_pos[1] + height / 2 * ly[i]);
		posList[3].push_back(0.0f);
		posList[3].push_back(r);
		posList[3].push_back(g);
		posList[3].push_back(b);
	}
	
	index[3].push_back(lastindex);
	index[3].push_back(lastindex + 1);
	index[3].push_back(lastindex + 2);
	index[3].push_back(lastindex + 1);
	index[3].push_back(lastindex + 2);
	index[3].push_back(lastindex + 3);

}

float random_float(float low, float high) {
	return low + (float)rand() * (high - low) / RAND_MAX;
}