#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h> 
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <math.h>
#include <vector>

#define MAXSHAPECOUNT 10

//--- 필요한 헤더파일 선언
//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);
GLvoid init_buffer();
void timer_move(int value);
void draw_shapes();
void input_rect(GLfloat* input_pos);
float random_float(float low, float high);
GLvoid Keyboard(unsigned char key, int x, int y);
void spckeycallback(int key, int x, int y);
void Mouse(int button, int state, int x, int y);
void clamp_pos(GLfloat* input_pos);
void input_shape(char cmd, GLfloat* input_pos);
void input_tri(GLfloat* input_pos);
void input_line(GLfloat* input_pos);
void input_dot(GLfloat* input_pos);
void specialKeyUp(int key, int x, int y);
void random_shape();
void move_shape();

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VBO_dot, VBO_line, VBO_tri, VBO_rect, EBO;
std::vector<GLuint> VAO;
std::vector<GLuint> VBO;
char cmd = 0;
int shape[4] = { GL_POINTS, GL_LINES,  GL_TRIANGLES , GL_TRIANGLES };
int rnd_shape = 0, rnd_index = 0;
int dir = 0;

int shape_counts[4] = { 0, };
int shape_count = 0;
int iskeydown = 0;

std::vector<std::vector<unsigned int>> index = {
	{}, {}, {}, {}
};



std::vector <std::vector<float >> posList = { {}, {}, {},
	{}
};


//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	srand((unsigned int)time(NULL));
	width = 800;
	height = 800;
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderID = make_shaderProgram();
	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutSpecialFunc(spckeycallback);
	glutSpecialUpFunc(specialKeyUp);
	init_buffer();
	glutMainLoop();
}

void reset() {
	for(int i = 0; i < 4; i++) shape_counts[i] = 0;
	shape_count = 0;
	iskeydown = 0;
	index.clear();
	index = { {}, {}, {}, {} };
	posList.clear();
	posList = { {}, {}, {}, {} };
}

void timer_move(int value) {
	glutPostRedisplay();
	glutTimerFunc(100, timer_move, 1);
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

	case 'c':
		reset();
		break;
	}
	glutPostRedisplay();
}

void spckeycallback(int key, int x, int y) {
	if (!iskeydown) {
		switch (key) {
		case GLUT_KEY_LEFT:
			dir = 2;
			break;

		case GLUT_KEY_RIGHT:
			dir = 0;
			break;

		case GLUT_KEY_UP:
			dir = 3;
			break;

		case GLUT_KEY_DOWN:
			dir = 1;
			break;
		default:
			dir = -1;
		}
		random_shape();
		iskeydown = 1;
	}
	move_shape();
}

void specialKeyUp(int key, int x, int y) {
	iskeydown = 0;
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
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
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
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
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
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
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
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
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
	if (shape_count >= MAXSHAPECOUNT) return;
	switch (cmd) {
	case 'p':
		input_dot(input_pos);
		break;
	case 'l':
		input_line(input_pos);
		break;

	case 't':
		input_tri(input_pos);
		break;

	case 'r':
		input_rect(input_pos);
		break;
	}
	shape_count++;
}

void input_dot(GLfloat* input_pos) {

	float lx[4] = { -1, -1, 1, 1 };
	float ly[4] = { -1, 1, -1, 1 };

	float width = 0.01f;
	float height = 0.01f;

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

	shape_counts[3]++;
}

void input_line(GLfloat* input_pos) {
	float lx[2] = { -1, 1};
	float ly[2] = {0, 0};

	float lenghth = 0.3f;

	float r = random_float(0.3, 1);
	float g = random_float(0.3, 1);
	float b = random_float(0.3, 1);

	int lastindex = index[1].size() / 2 * 2;

	for (int i = 0; i < 2; i++) {
		posList[1].push_back(input_pos[0] + lenghth / 2 * lx[i]);
		posList[1].push_back(input_pos[1] + lenghth / 2 * ly[i]);
		posList[1].push_back(0.0f);
		posList[1].push_back(r);
		posList[1].push_back(g);
		posList[1].push_back(b);
	}

	index[1].push_back(lastindex);
	index[1].push_back(lastindex + 1);
	shape_counts[1]++;
}

void input_tri(GLfloat* input_pos) {
	float lx[3] = { 0, -0.5, 0.5};
	float ly[3] = { 0.5, -0.5, -0.5};

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
	shape_counts[2]++;
}

void input_rect(GLfloat* input_pos) {
	float lx[4] = { -1, -1, 1, 1 };
	float ly[4] = { -1, 1, -1, 1 };

	float width = 0.2f;
	float height = 0.2f;

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
	shape_counts[3]++;
}

float random_float(float low, float high) {
	return low + (float)rand() * (high - low) / RAND_MAX;
}

void random_shape() {
	rnd_shape = rand() % 4;

	while (shape_counts[rnd_shape] == 0) {
		rnd_shape = rand() % 4;
	}
	rnd_index = rand() % shape_counts[rnd_shape];

}

void move_shape() {
	if (dir == -1)return;
	if (shape_count == 0) return;
	int vertex_count[4] = { 4, 2, 3, 4 };
	int dx[4] = { 1, 0, -1, 0 };
	int dy[4] = { 0, -1, 0, 1 };
	for (int i = 0; i < vertex_count[rnd_shape]; i++) {

		posList[rnd_shape][6 * (rnd_index * vertex_count[rnd_shape] + i)] += dx[dir] * 0.01f;
		posList[rnd_shape][6 * (rnd_index * vertex_count[rnd_shape] + i) + 1] += dy[dir] * 0.01f;
	}
	glutPostRedisplay();
}
