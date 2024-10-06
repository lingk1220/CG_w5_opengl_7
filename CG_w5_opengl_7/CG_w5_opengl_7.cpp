#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h> 
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <math.h>
#include <vector>

//--- 필요한 헤더파일 선언
//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);
GLvoid init_buffer();
void timer(int value);
void draw_shapes();
void input_rect();
float random_float(float low, float high);
//--- 필요한 변수 선언
GLint width, height;
GLuint shaderID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VBO_dot, VBO_line, VBO_tri, VBO_rect, EBO;
std::vector<GLuint> VAO;
std::vector<GLuint> VBO;
float offset = 0;

int shape[4] = { GL_POINTS, GL_LINES,  GL_TRIANGLES , GL_TRIANGLES };
std::vector<std::vector<unsigned int>> index = {
	{}, {}, {}, {
0, 1, 3, // 첫 번째 삼각형
1, 2, 3, // 두 번째 삼각형
4, 5, 7,
5, 6, 7 }
};

const GLfloat colors[3][3] = { // 삼각형 꼭지점 색상
{1.0, 0.0, 0.0},
{0.0, 1.0, 0.0},
{0.0, 0.0, 1.0} };

std::vector <std::vector<float >> posList = { {}, {}, {},
	{}
};


//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	srand((unsigned int)time(NULL));
	width = 500;
	height = 500;
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
	init_buffer();

	glutTimerFunc(1000, timer, 1);
	glutMainLoop();
}

void timer(int value) {
	input_rect();
	glutPostRedisplay();
	glutTimerFunc(1000, timer, 1);
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
		glGenBuffers(1, &EBO);
	}
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

void test() {
	
}

void input_rect() {
	float lx[4] = { 0, 1, 0, 1 };
	float ly[4] = { 0, 0, 1, 1 };

	float ox = random_float(-1.0, 1.0);
	float oy = random_float(-1.0, 1.0);

	std::cout << ox << " " << oy << "\n";
	float width = random_float(0.3, 0.5);
	float height = random_float(0.3, 0.5);

	float r = random_float(0.3, 1);
	float g = random_float(0.3, 1);
	float b = random_float(0.3, 1);

	int lastindex = index[2].size() / 6 * 4;

	for (int i = 0; i < 4; i++) {
		posList[2].push_back(ox + width * lx[i]);
		posList[2].push_back(oy + height * ly[i]);
		posList[2].push_back(0.0f);
		posList[2].push_back(r);
		posList[2].push_back(g);
		posList[2].push_back(b);
	}
	
	index[2].push_back(lastindex);
	index[2].push_back(lastindex + 1);
	index[2].push_back(lastindex + 2);
	index[2].push_back(lastindex + 1);
	index[2].push_back(lastindex + 2);
	index[2].push_back(lastindex + 3);

}

float random_float(float low, float high) {
	std::cout << (float)rand() * (high - low) / RAND_MAX << "\n";
	return low + (float)rand() * (high - low) / RAND_MAX;
}