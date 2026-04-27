#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include "spheremesh.h"

#define PI glm::pi<float>()

GLuint shader_programme, vao;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

glm::vec3 lightPos(0, 1, 5);
glm::vec3 viewPos(2, 3, 6);

SphereMesh sphere(1);
int sphereElementCount = (GLsizei)sphere.triangles.size() * sizeof(glm::ivec3);

std::string textFileRead(const char *fn)
{
	std::ifstream ifile(fn);
	std::string filetext;
	while (ifile.good()) {
		std::string line;
		std::getline(ifile, line);
		// Strip Windows-style \r if present
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		filetext.append(line + "\n");
	}
	return filetext;
}

void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

float rotAngle    = 0;
float rotAngleInc = PI / 64;

// Modul curent: 1 = iluminare per-vertex, 2 = iluminare per-pixel
int currentMode = 1;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader_programme);

	modelMatrix = glm::mat4(1.0f);

	glBindVertexArray(vao);
	
	GLuint lightPosLoc = glGetUniformLocation(shader_programme, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	GLuint viewPosLoc = glGetUniformLocation(shader_programme, "viewPos");
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

	modelMatrix *= glm::rotate(rotAngle, glm::vec3(0, 1, 0));
	GLuint modelMatrixLoc = glGetUniformLocation(shader_programme, "mvpMatrix");
	glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));

	// Matricea de corectie a normalelor: transpusa inversei matricii de modelare
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
	GLuint normalMatrixLoc = glGetUniformLocation(shader_programme, "normalMatrix");
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// Trimite modul curent (1=per-vertex, 2=per-pixel)
	GLuint modeLoc = glGetUniformLocation(shader_programme, "mode");
	glUniform1i(modeLoc, currentMode);

	glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);

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
	glEnable(GL_DEPTH_TEST);

	glewInit();

	GLuint vbo, ebo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(glm::vec3), sphere.vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereElementCount, sphere.triangles.data(), GL_STATIC_DRAW);


	std::string vstext = textFileRead("pixel_light.vert");
	std::string fstext = textFileRead("pixel_light.frag");
		
	const char* vertex_shader = vstext.c_str();
	const char* fragment_shader = fstext.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	printShaderInfoLog(vs);
	printShaderInfoLog(fs);
	printProgramInfoLog(shader_programme);

	printf("Taste:\n");
	printf("  a/s - rotatie stanga/dreapta\n");
	printf("  1   - iluminare per-vertex\n");
	printf("  2   - iluminare per-pixel\n");
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	projectionMatrix = glm::perspective(PI / 6, (float)w / h, 0.1f, 100.0f);
	/*
	viewMatrix este matricea transformarii de observare. Parametrii functiei
	lookAt sunt trei vectori ce reprezinta, in ordine:
	- pozitia observatorului
	- punctul catre care priveste observatorul
	- directia dupa care este orientat observatorul
	*/
	viewMatrix = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		rotAngle += rotAngleInc;
		break;
	case 's':
		rotAngle -= rotAngleInc;
		break;
	case '1':
		currentMode = 1;
		printf("Mode 1: iluminare per-vertex\n");
		break;
	case '2':
		currentMode = 2;
		printf("Mode 2: iluminare per-pixel\n");
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(700, 700);
	glutCreateWindow("SPG Lab8 - Iluminare Sfera (Phong)");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
