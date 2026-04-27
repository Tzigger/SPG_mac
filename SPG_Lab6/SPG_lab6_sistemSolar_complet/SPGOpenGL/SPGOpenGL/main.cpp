#include <fstream>
#include <string>
#include <stack>
#include <libgen.h>   // dirname
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include "spheremesh.h"

#define PI glm::pi<float>()

// Directory of the executable – set in main()
static std::string g_baseDir;

GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;
std::stack<glm::mat4> modelStack;

GLuint vboAxes, vaoAxes;
GLuint vboSphere, vaoSphere;
GLuint eboSphere; //index buffer pentru sfera

float axes[] = {
	//axele de coordonate
	0.0f, 0.0f, 0.0f,
	16.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 8.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 16.0f
};

SphereMesh sphere;
int sphereElementCount = (GLsizei)sphere.triangles.size() * sizeof(glm::ivec3);

float xv = 10, yv = 12, zv = 30; //originea sistemului de observare

std::string textFileRead(const char *fn)
{
	std::ifstream ifile(fn);
	if (!ifile.good() && !g_baseDir.empty())
		ifile.open(g_baseDir + "/" + fn);
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

void init()
{
	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glewInit();

	glGenBuffers(1, &vboAxes);
	glBindBuffer(GL_ARRAY_BUFFER, vboAxes);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), axes, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vaoAxes);
	glBindVertexArray(vaoAxes);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glGenBuffers(1, &vboSphere);
	glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(glm::vec3), sphere.vertices.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glGenBuffers(1, &eboSphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereElementCount, sphere.triangles.data(), GL_STATIC_DRAW);

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
	glBindAttribLocation(shader_programme, 0, "vp");
	glLinkProgram(shader_programme);
	checkProgram(shader_programme);
}

struct Planet
{
	float radius; // raza planetei
	float axisRotAngle; // unghiul de rotatie in jurul propriei axe
	float axisRotAngleInc; // valoarea cu care se incrementeaza unghiul (de ex la apasarea unei taste)
	float orbitDist; // distanta fata de centrul orbitei
	float orbitAngle; // unghiul de rotatie pe orbita
	float orbitAngleInc; // valoarea cu care se incrementeaza unghiul (de ex la apasarea unei taste)

	Planet(
		float _radius, float _axisRotAngle, float _axisRotAngleInc,
		float _orbitDist, float _orbitAngle, float _orbitAngleInc) :
		radius(_radius), axisRotAngle(_axisRotAngle),
		axisRotAngleInc(_axisRotAngleInc), orbitDist(_orbitDist),
		orbitAngle(_orbitAngle), orbitAngleInc(_orbitAngleInc)
	{}
};

Planet sun(2.0f, 0, PI / 128, 0, 0, 0);
Planet p1(0.7f, 0, PI / 16, 8.0, PI / 8, PI / 32);
Planet p1s1(0.3f, 0, PI / 32, 3.0, PI / 16, PI / 16);
Planet p1s1s1(0.12f, 0, PI / 20, 1.2f, PI / 8, PI / 10); // satelit al satelitului p1s1

Planet p2(0.9f, 0, PI / 20, 14.0f, PI / 4, PI / 48);     // a doua planeta
Planet p2s1(0.35f, 0, PI / 28, 3.5f, PI / 6, PI / 20);   // satelit al planetei p2

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader_programme);

	GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
	GLuint colorID = glGetUniformLocation(shader_programme, "color");

	modelMatrix = glm::mat4(); // matricea de modelare este matricea identitate

							   //desenare axe coordonate
	glBindVertexArray(vaoAxes);
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
	glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(vaoSphere);

	//planeta centrala (soarele)
	modelStack.push(modelMatrix); // se salveaza pe stiva modelMatrix, deoarece urmatoarele doua transformari se aplica doar soarelui
		modelMatrix *= glm::rotate(sun.axisRotAngle, glm::vec3(0, 1, 0)); // rotatia soarelui in jurul propriei axe
		modelMatrix *= glm::scale(glm::vec3(2 * sun.radius)); // scalarea care defineste dimensiunea soarelui
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top(); // se readuce modelMatrix la valoarea din momentul in care s-a facut push()
	modelStack.pop(); // se elimina din stiva valoarea curenta a modelMatrix

					  // urmatoarele doua transformari definesc rotatia pe orbita a primei planete
					  //	transformarile se aplica atat planetei, cat si satelitului sau

	modelMatrix *= glm::rotate(p1.orbitAngle, glm::vec3(0, 1, 0));
	modelMatrix *= glm::translate(glm::vec3(p1.orbitDist, 0, 0));

	modelStack.push(modelMatrix); //se salveaza pe stiva modelMatrix, deoarece urmatoarele doua transformari se aplica doar planetei, nu si satelitului sau
		modelMatrix *= glm::rotate(p1.axisRotAngle, glm::vec3(0, 1, 0)); // rotatia planetei in jurul propriei axe
		modelMatrix *= glm::scale(glm::vec3(2 * p1.radius)); // scalarea care defineste dimensiunea planetei
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top(); // se readuce modelMatrix la valoarea din momentul in care s-a facut push()
	modelStack.pop(); // se elimina din stiva valoarea curenta a modelMatrix

					  // urmatoarele doua transformari definesc rotatia pe orbita a satelitului primei planete

	modelMatrix *= glm::rotate(p1s1.orbitAngle, glm::vec3(0, 1, 0));
	modelMatrix *= glm::translate(glm::vec3(p1s1.orbitDist, 0, 0));

	//urmeaza doua transformari care se aplica exclusiv satelitului p1s1
	modelStack.push(modelMatrix);
		modelMatrix *= glm::rotate(p1s1.axisRotAngle, glm::vec3(0, 1, 0)); // rotatia satelitului in jurul propriei axe
		modelMatrix *= glm::scale(glm::vec3(2 * p1s1.radius)); // scalarea care defineste dimensiunea satelitului
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top();
	modelStack.pop();

	// satelit al satelitului p1s1 (p1s1s1)
	modelMatrix *= glm::rotate(p1s1s1.orbitAngle, glm::vec3(0, 1, 0));
	modelMatrix *= glm::translate(glm::vec3(p1s1s1.orbitDist, 0, 0));

	modelStack.push(modelMatrix);
		modelMatrix *= glm::rotate(p1s1s1.axisRotAngle, glm::vec3(0, 1, 0));
		modelMatrix *= glm::scale(glm::vec3(2 * p1s1s1.radius));
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(0.6f, 0.2f, 1.0f)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top();
	modelStack.pop();

	// A doua planeta (p2) si satelitul sau (p2s1)
	modelMatrix = glm::mat4();

	modelMatrix *= glm::rotate(p2.orbitAngle, glm::vec3(0, 1, 0));
	modelMatrix *= glm::translate(glm::vec3(p2.orbitDist, 0, 0));

	modelStack.push(modelMatrix);
		modelMatrix *= glm::rotate(p2.axisRotAngle, glm::vec3(0, 1, 0));
		modelMatrix *= glm::scale(glm::vec3(2 * p2.radius));
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(0.0f, 0.8f, 0.4f)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top();
	modelStack.pop();

	// p2s1
	modelMatrix *= glm::rotate(p2s1.orbitAngle, glm::vec3(0, 1, 0));
	modelMatrix *= glm::translate(glm::vec3(p2s1.orbitDist, 0, 0));

	modelStack.push(modelMatrix);
		modelMatrix *= glm::rotate(p2s1.axisRotAngle, glm::vec3(0, 1, 0));
		modelMatrix *= glm::scale(glm::vec3(2 * p2s1.radius));
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
		glUniform3fv(colorID, 1, glm::value_ptr(glm::vec3(1.0f, 0.6f, 0.0f)));
		glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);
	modelMatrix = modelStack.top();
	modelStack.pop();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	projectionMatrix = glm::perspective(PI / 4, (float)w / h, 0.1f, 100.0f);
	/*
	viewMatrix este matricea transformarii de observare. Parametrii functiei
	lookAt sunt trei vectori ce reprezinta, in ordine:
	- pozitia observatorului
	- punctul catre care priveste observatorul
	- directia dupa care este orientat observatorul
	*/
	viewMatrix = glm::lookAt(glm::vec3(xv, yv, zv), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void keyboard(unsigned char key, int x, int y)
{
	float dir = 0.0f;
	if (key == 'a') dir = 1.0f;
	else if (key == 's') dir = -1.0f;
	else return;

	// soare - rotatie in jurul propriei axe
	sun.axisRotAngle += dir * sun.axisRotAngleInc;
	if (sun.axisRotAngle > 2 * PI) sun.axisRotAngle -= 2 * PI;
	if (sun.axisRotAngle < 0) sun.axisRotAngle += 2 * PI;

	// planeta 1 - rotatie proprie + pe orbita
	p1.axisRotAngle += dir * p1.axisRotAngleInc;
	if (p1.axisRotAngle > 2 * PI) p1.axisRotAngle -= 2 * PI;
	if (p1.axisRotAngle < 0) p1.axisRotAngle += 2 * PI;

	p1.orbitAngle += dir * p1.orbitAngleInc;
	if (p1.orbitAngle > 2 * PI) p1.orbitAngle -= 2 * PI;
	if (p1.orbitAngle < 0) p1.orbitAngle += 2 * PI;

	// satelit 1 al planetei 1 - rotatie proprie + pe orbita
	p1s1.axisRotAngle += dir * p1s1.axisRotAngleInc;
	if (p1s1.axisRotAngle > 2 * PI) p1s1.axisRotAngle -= 2 * PI;
	if (p1s1.axisRotAngle < 0) p1s1.axisRotAngle += 2 * PI;

	p1s1.orbitAngle += dir * p1s1.orbitAngleInc;
	if (p1s1.orbitAngle > 2 * PI) p1s1.orbitAngle -= 2 * PI;
	if (p1s1.orbitAngle < 0) p1s1.orbitAngle += 2 * PI;

	// satelit al satelitului p1s1 - rotatie proprie + pe orbita
	p1s1s1.axisRotAngle += dir * p1s1s1.axisRotAngleInc;
	if (p1s1s1.axisRotAngle > 2 * PI) p1s1s1.axisRotAngle -= 2 * PI;
	if (p1s1s1.axisRotAngle < 0) p1s1s1.axisRotAngle += 2 * PI;

	p1s1s1.orbitAngle += dir * p1s1s1.orbitAngleInc;
	if (p1s1s1.orbitAngle > 2 * PI) p1s1s1.orbitAngle -= 2 * PI;
	if (p1s1s1.orbitAngle < 0) p1s1s1.orbitAngle += 2 * PI;

	// planeta 2 - rotatie proprie + pe orbita
	p2.axisRotAngle += dir * p2.axisRotAngleInc;
	if (p2.axisRotAngle > 2 * PI) p2.axisRotAngle -= 2 * PI;
	if (p2.axisRotAngle < 0) p2.axisRotAngle += 2 * PI;

	p2.orbitAngle += dir * p2.orbitAngleInc;
	if (p2.orbitAngle > 2 * PI) p2.orbitAngle -= 2 * PI;
	if (p2.orbitAngle < 0) p2.orbitAngle += 2 * PI;

	// satelit al planetei 2 - rotatie proprie + pe orbita
	p2s1.axisRotAngle += dir * p2s1.axisRotAngleInc;
	if (p2s1.axisRotAngle > 2 * PI) p2s1.axisRotAngle -= 2 * PI;
	if (p2s1.axisRotAngle < 0) p2s1.axisRotAngle += 2 * PI;

	p2s1.orbitAngle += dir * p2s1.orbitAngleInc;
	if (p2s1.orbitAngle > 2 * PI) p2s1.orbitAngle -= 2 * PI;
	if (p2s1.orbitAngle < 0) p2s1.orbitAngle += 2 * PI;

	glutPostRedisplay(); // cauzeaza redesenarea ferestrei
}

int main(int argc, char** argv)
{
	char* exePath = strdup(argv[0]);
	g_baseDir = std::string(dirname(exePath));
	free(exePath);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(700, 700);
	glutCreateWindow("SPG - Sistem Solar");
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
