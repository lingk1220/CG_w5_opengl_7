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

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VBO_dot, VBO_line, VBO_tri, VBO_rect, EBO;
std::vector<GLuint> VAO;
std::vector<GLuint> VBO;
float offset = 0;

int shape[4] = { GL_POINTS, GL_LINES,  GL_TRIANGLES , GL_TRIANGLES };
std::vector<std::vector<unsigned int>> index = {
	{}, {}, {}, {
0, 1, 3, // ù ��° �ﰢ��
1, 2, 3, // �� ��° �ﰢ��
4, 5, 7,
5, 6, 7 }
};

const GLfloat colors[3][3] = { // �ﰢ�� ������ ����
{1.0, 0.0, 0.0},
{0.0, 1.0, 0.0},
{0.0, 0.0, 1.0} };

std::vector <std::vector<float >> posList = { {}, {}, {},
	{1.0f, 1.0f, 0.0f, // ���� ���
1.0f, 0.0f, 0.0f, // ���� �ϴ�
0.0f, 0.0f, 0.0f, // ���� �ϴ�
0.0f, 1.0f, 0.0f, // ���� ���
-0.5f, -0.5f, 0.0f,
-0.5f, -1.0f, 0.0f,
-1.0f, -1.0f, 0.0f,
-1.0f, -0.5f, 0.0f}
};


//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	width = 500;
	height = 500;
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
	init_buffer();
	glutMainLoop();
}

void timer(int value) {
	float randomvalue = ((float)(rand() % 256) - 128) / 128;
	offset += randomvalue / 200.0f;
	std::cout << sin(randomvalue) / 200.0f << std::endl;

	int vertexOffsetLocation = glGetUniformLocation(shaderID, "offset");
	glUniform1f(vertexOffsetLocation, offset);
	glutPostRedisplay();
	glutTimerFunc(10, timer, 1);
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
	gColor = 1.0;
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
}

void draw_shapes() {
	for (int i = 0; i < 4; i++) {
		if (posList.empty()) continue;
		glGenVertexArrays(1, &VAO[i]);
		glGenBuffers(1, &VBO[i]);
		glBindVertexArray(VAO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, posList[i].size() * sizeof(float), posList[i].data(), GL_STATIC_DRAW);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index[i].size() * sizeof(float), index[i].data(), GL_STATIC_DRAW);
		std::cout << posList.size() << std::endl;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glDrawElements(shape[i], index[i].size(), GL_UNSIGNED_INT, 0);
	}
}