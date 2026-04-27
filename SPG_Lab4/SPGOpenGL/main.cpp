#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//varfurile triunghiului
float points[] = {
	0.0f,  0.5f,  0.0f,
	0.5f, -0.5f,  0.0f,
	-0.5f, -0.5f,  0.0f
};

GLuint shader_programme, vao;
glm::mat4 model;
std::string g_asset_dir = ".";

std::string textFileRead(const char *fn)
{
	std::ifstream ifile(fn);
	if (!ifile.is_open()) {
		std::string fallback = g_asset_dir + "/" + fn;
		ifile.open(fallback.c_str());
		if (!ifile.is_open()) {
			fprintf(stderr, "Could not open shader file: %s (also tried %s)\n", fn, fallback.c_str());
			return "";
		}
	}
	std::string filetext;
	while (ifile.good()) {
		std::string line;
		std::getline(ifile, line);
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		filetext.append(line + "\n");
	}
	return filetext;
}

bool checkShader(GLuint shader, const char* label)
{
	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		return true;
	}

	GLint logLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		std::string log(logLen, '\0');
		glGetShaderInfoLog(shader, logLen, NULL, &log[0]);
		fprintf(stderr, "%s compile error:\n%s\n", label, log.c_str());
	}
	return false;
}

bool checkProgram(GLuint program)
{
	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_TRUE) {
		return true;
	}

	GLint logLen = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		std::string log(logLen, '\0');
		glGetProgramInfoLog(program, logLen, NULL, &log[0]);
		fprintf(stderr, "Program link error:\n%s\n", log.c_str());
	}
	return false;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_programme);

	float rotationAngle = glm::pi<float>() / 2;

	glm::mat4 rotationMatrix{  glm::cos(rotationAngle), glm::sin(rotationAngle), 0, 0,
							  -glm::sin(rotationAngle), glm::cos(rotationAngle), 0, 0,
								0,0,1,0, 
								0,0,0,1 
							};

	GLuint matrixID = glGetUniformLocation(shader_programme, "modelMatrix");
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(rotationMatrix));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);

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
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	std::string vstext = textFileRead("vertex.vert");
	std::string fstext = textFileRead("fragment.frag");
	if (vstext.empty() || fstext.empty()) {
		fprintf(stderr, "Shader sources are empty. Run app from shader directory.\n");
	}
	const char* vertex_shader = vstext.c_str();
	const char* fragment_shader = fstext.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	if (!checkShader(vs, "Vertex shader")) {
		return;
	}
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	if (!checkShader(fs, "Fragment shader")) {
		return;
	}

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);
	if (!checkProgram(shader_programme)) {
		return;
	}
}

int main(int argc, char** argv)
{
	if (argc > 0 && argv[0] != NULL) {
		std::string exePath(argv[0]);
		size_t lastSlash = exePath.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			g_asset_dir = exePath.substr(0, lastSlash);
		}
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(512, 512);
	glutCreateWindow("SPG");

	init();

	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
