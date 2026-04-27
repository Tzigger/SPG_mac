#include <fstream>
#include <string>
#include <stack>
#include <libgen.h>   // dirname

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // strdup

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

// Directory of the executable – set in main() so shader files are always found
static std::string g_baseDir;

#define PI glm::pi<float>()

GLuint shader_programme, vao;
glm::mat4 projectionMatrix, viewMatrix;

std::stack <glm::mat4> modelStack;

float segment1 = 2; //lungimea bratului
float segment1Angle = PI / 12; //unghiul de rotatie al bratului
float segment2 = 3; //lungimea antebratului
float segment2Angle = PI / 8; //unghiul de rotatie al antebratului

float fingerLength = 0.8f; //lungimea unui deget
float fingerAngle = PI / 8; //unghiul de forfecare al degetelor (fata de axa antebratului)

float points[] = {
	-0.5f,  -0.5f,  -0.5f,
	-0.5f, 0.5f,  -0.5f,
	-0.5f, 0.5f,  -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f,  -0.5f,
	0.5f, -0.5f,  -0.5f,
	- 0.5f,  -0.5f,  -0.5f,

	-0.5f,  -0.5f,  0.5f,
	-0.5f, 0.5f,  0.5f,
	-0.5f, 0.5f,  0.5f,
	0.5f,  0.5f, 0.5f,
	0.5f,  0.5f, 0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f,  -0.5f,  0.5f,

	-0.5f,  -0.5f,  0.5f,
	-0.5f,  -0.5f,  -0.5f,
	-0.5f, 0.5f,  0.5f,
	-0.5f, 0.5f,  -0.5f,
	0.5f,  0.5f, 0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  -0.5f,

	0.0f, 0.0f, 0.0f,
	16.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 8.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 24.0f,
};

std::string textFileRead(const char *fn)
{
	// Try the path as-is first; if it fails, try relative to the exe directory
	std::ifstream ifile(fn);
	if (!ifile.good() && !g_baseDir.empty()) {
		ifile.open(g_baseDir + "/" + fn);
	}
	if (!ifile.good()) {
		fprintf(stderr, "textFileRead: cannot open '%s'\n", fn);
		return "";
	}
	std::string filetext;
	while (ifile.good()) {
		std::string line;
		std::getline(ifile, line);
		filetext.append(line + "\n");
	}
	return filetext;
}

static void checkShader(GLuint shader, const char* name)
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char buf[1024];
		glGetShaderInfoLog(shader, sizeof(buf), nullptr, buf);
		fprintf(stderr, "Shader compile error [%s]:\n%s\n", name, buf);
	}
}

static void checkProgram(GLuint prog)
{
	GLint status = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (!status) {
		char buf[1024];
		glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
		fprintf(stderr, "Program link error:\n%s\n", buf);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_programme);

	float rotationAngle = PI / 2;
	glm::mat4 modelMatrix;

	glBindVertexArray(vao);
	GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");

	////desenare axe coordonate
	
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
	glDrawArrays(GL_LINES, 24, 6);

	//brat
	modelMatrix *= glm::rotate(segment1Angle, glm::vec3(0, 0, 1));
	modelMatrix *= glm::translate(glm::vec3(segment1 / 2.0f, 0, 0));
	
	modelStack.push(modelMatrix);
		modelMatrix *= glm::scale(glm::vec3(segment1, 0.5, 0.5));

		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glDrawArrays(GL_LINES, 0, 24);
	modelMatrix = modelStack.top();
	modelStack.pop();

	////antebrat
	modelMatrix *= glm::translate(glm::vec3(segment1 / 2.0f, 0, 0));
	modelMatrix *= glm::rotate(segment2Angle, glm::vec3(0, 0, 1));
	modelMatrix *= glm::translate(glm::vec3(segment2 / 2.0f, 0, 0));

	modelStack.push(modelMatrix);
		modelMatrix *= glm::scale(glm::vec3(segment2, 0.5, 0.5));

		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glDrawArrays(GL_LINES, 0, 24);
	modelMatrix = modelStack.top();
	modelStack.pop();

	////degete
	modelMatrix *= glm::translate(glm::vec3(segment2 / 2.0f, 0, 0)); // capatul antebratului.
	modelStack.push(modelMatrix); 

	// Deget 1
	modelMatrix *= glm::rotate(fingerAngle, glm::vec3(0, 0, 1));
	modelMatrix *= glm::translate(glm::vec3(fingerLength / 2.0f, 0, 0));
	modelStack.push(modelMatrix);
		modelMatrix *= glm::scale(glm::vec3(fingerLength, 0.25f, 0.25f));
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glDrawArrays(GL_LINES, 0, 24);
	modelMatrix = modelStack.top();
	modelStack.pop();

	// Deget 2 
	modelMatrix = modelStack.top();
	modelMatrix *= glm::rotate(-fingerAngle, glm::vec3(0, 0, 1));
	modelMatrix *= glm::translate(glm::vec3(fingerLength / 2.0f, 0, 0));
	modelStack.push(modelMatrix);
		modelMatrix *= glm::scale(glm::vec3(fingerLength, 0.25f, 0.25f));
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glDrawArrays(GL_LINES, 0, 24);
	modelMatrix = modelStack.top();
	modelStack.pop();



	glFlush();
}

void init()
{
	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glClearColor(1, 1, 1, 0);

	glewInit();

	GLuint vbo = 1;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 90 * sizeof(float), points, GL_STATIC_DRAW);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	std::string vstext = textFileRead("vertex.vert");
	std::string fstext = textFileRead("fragment.frag");
	const char* vertex_shader = vstext.c_str();
	const char* fragment_shader = fstext.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	checkShader(vs, "vertex");

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	checkShader(fs, "fragment");

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	// Bind 'vp' to attribute location 0 (matches glVertexAttribPointer index 0)
	glBindAttribLocation(shader_programme, 0, "vp");
	glLinkProgram(shader_programme);
	checkProgram(shader_programme);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 's':
		segment1Angle += 0.1;
			break;
	case 'd':
		segment1Angle -= 0.1;
			break;
	case 'x':
		segment2Angle += 0.1;
		break;
	case 'c':
		segment2Angle -= 0.1;
		break;
	case 'z': // deschide degetele (forfecare mai mare)
		fingerAngle += 0.05f;
		if (fingerAngle > PI / 2) fingerAngle = PI / 2;
		break;
	case 'a': // inchide degetele (forfecare mai mica)
		fingerAngle -= 0.05f;
		if (fingerAngle < 0) fingerAngle = 0;
		break;
	}
	glutPostRedisplay();
}


void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	projectionMatrix = glm::perspective(PI / 3, (float)w / h, 0.1f, 100.0f);
	/*
	viewMatrix este matricea transformarii de observare. Parametrii functiei
	lookAt sunt trei vectori ce reprezinta, in ordine:
	- pozitia observatorului
	- punctul catre care priveste observatorul
	- directia dupa care este orientat observatorul
	*/
	viewMatrix = glm::lookAt(glm::vec3(0, 0, 10),	glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

int main(int argc, char** argv)
{
	// Resolve the directory containing the executable so shader files are found
	// regardless of the working directory.
	char* exePath = strdup(argv[0]);
	g_baseDir = std::string(dirname(exePath));
	free(exePath);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(700, 700);
	glutCreateWindow("SPG - Brat Robot");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}
