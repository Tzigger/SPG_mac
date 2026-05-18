#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

// Comanda simpla de desen: ce tip de primitiva desenam si ce interval din VBO folosim
struct DrawCommand {
	GLenum mode;
	GLint first;
	GLsizei count;
	float pointSize;
};

GLuint shader_programme = 0;
GLuint vao = 0;
GLuint vbo = 0;
glm::mat4 projectionMatrix(1.0f), viewMatrix(1.0f);
int currentExercise = 1;
int gWindowW = 1000;
int gWindowH = 800;

// Aici punem toate varfurile (x,y,z) care ajung in VBO
std::vector<float> points;
// Lista de "ce desenam" din vectorul points
std::vector<DrawCommand> drawCommands;

const unsigned int LOD_HERMITE = 120;
const unsigned int LOD_BEZIER = 120;
const unsigned int LOD_EX1 = 120;
const unsigned int LOD_BSPLINE = 160;

void updateProjection(int w, int h);
void createPointsVector();

std::string textFileRead(const char* fn)
{
	std::ifstream ifile(fn);
	std::string filetext;
	while (ifile.good()) {
		std::string line;
		std::getline(ifile, line);
		filetext.append(line + "\n");
	}
	return filetext;
}

GLuint compileShader(GLenum type, const char* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		std::vector<char> log(logLen + 1);
		glGetShaderInfoLog(shader, logLen, NULL, log.data());
		std::printf("Shader compile error: %s\n", log.data());
	}

	return shader;
}

glm::vec3 hermitePoint(float u, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& t0, const glm::vec3& t1)
{
	// u in [0,1], calculam functiile de amestec Hermite/Coons
	float u2 = u * u;
	float u3 = u2 * u;

	float f0 = 2.0f * u3 - 3.0f * u2 + 1.0f;
	float f1 = -2.0f * u3 + 3.0f * u2;
	float f2 = u3 - 2.0f * u2 + u;
	float f3 = u3 - u2;

	// P(u) = f0*P0 + f1*P1 + f2*T0 + f3*T1
	return f0 * p0 + f1 * p1 + f2 * t0 + f3 * t1;
}

glm::vec3 bezierPoint(float u, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	// Functii Bernstein pentru Bezier cubic
	float oneMinusU = 1.0f - u;
	float oneMinusU2 = oneMinusU * oneMinusU;
	float oneMinusU3 = oneMinusU2 * oneMinusU;
	float u2 = u * u;
	float u3 = u2 * u;

	float b0 = oneMinusU3;
	float b1 = 3.0f * u * oneMinusU2;
	float b2 = 3.0f * u2 * oneMinusU;
	float b3 = u3;

	// P(u) = b0*P0 + b1*P1 + b2*P2 + b3*P3
	return b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;
}

float bsplineBasis(int i, int order, float t, const std::vector<float>& knots)
{
	// Cox - de Boor pentru functiile de baza N(i,order)
	if (order == 1) {
		bool inSpan = (knots[i] <= t && t < knots[i + 1]);
		bool lastSpan = (t == knots.back() && i + 1 == static_cast<int>(knots.size()) - 1);
		return (inSpan || lastSpan) ? 1.0f : 0.0f;
	}

	float left = 0.0f;
	float right = 0.0f;

	float leftDenom = knots[i + order - 1] - knots[i];
	if (leftDenom > 1e-6f) {
		left = (t - knots[i]) / leftDenom * bsplineBasis(i, order - 1, t, knots);
	}

	float rightDenom = knots[i + order] - knots[i + 1];
	if (rightDenom > 1e-6f) {
		right = (knots[i + order] - t) / rightDenom * bsplineBasis(i + 1, order - 1, t, knots);
	}

	return left + right;
}

std::vector<glm::vec3> sampleBSpline(const std::vector<glm::vec3>& controlPoints, int order, const std::vector<float>& knots, unsigned int lod)
{
	std::vector<glm::vec3> curve;
	if (controlPoints.empty() || knots.empty() || lod < 2) {
		return curve;
	}

	int n = static_cast<int>(controlPoints.size()) - 1;
	float tMin = knots[order - 1];
	float tMax = knots[n + 1];

	curve.reserve(lod);
	for (unsigned int s = 0; s < lod; ++s) {
		float alpha = static_cast<float>(s) / static_cast<float>(lod - 1);
		float t = tMin + (tMax - tMin) * alpha;

		// Evitam probleme numerice la capatul din dreapta
		if (s == lod - 1) {
			curve.push_back(controlPoints.back());
			continue;
		}

		glm::vec3 p(0.0f);
		for (int i = 0; i <= n; ++i) {
			float Ni = bsplineBasis(i, order, t, knots);
			p += Ni * controlPoints[i];
		}
		curve.push_back(p);
	}

	return curve;
}

glm::vec3 evaluateBSplinePoint(const std::vector<glm::vec3>& controlPoints, int order, const std::vector<float>& knots, float t)
{
	int n = static_cast<int>(controlPoints.size()) - 1;
	float tMax = knots[n + 1];

	// Capatul din dreapta il fixam explicit pe ultimul punct de control
	if (std::abs(t - tMax) < 1e-6f) {
		return controlPoints.back();
	}

	glm::vec3 p(0.0f);
	for (int i = 0; i <= n; ++i) {
		float Ni = bsplineBasis(i, order, t, knots);
		p += Ni * controlPoints[i];
	}
	return p;
}

std::vector<glm::vec3> sampleBSplineSegment(
	const std::vector<glm::vec3>& controlPoints,
	int order,
	const std::vector<float>& knots,
	float tStart,
	float tEnd,
	unsigned int lod)
{
	std::vector<glm::vec3> curve;
	if (lod < 2) return curve;

	curve.reserve(lod);
	for (unsigned int i = 0; i < lod; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(lod - 1);
		float t = tStart + (tEnd - tStart) * u;
		curve.push_back(evaluateBSplinePoint(controlPoints, order, knots, t));
	}
	return curve;
}

std::vector<glm::vec3> sampleHermite(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& t0, const glm::vec3& t1, unsigned int lod)
{
	std::vector<glm::vec3> curve;
	curve.reserve(lod);
	// Generam LOD puncte pe curba Hermite
	for (unsigned int i = 0; i < lod; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(lod - 1);
		curve.push_back(hermitePoint(u, p0, p1, t0, t1));
	}
	return curve;
}

std::vector<glm::vec3> sampleBezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, unsigned int lod)
{
	std::vector<glm::vec3> curve;
	curve.reserve(lod);
	// Generam LOD puncte pe curba Bezier
	for (unsigned int i = 0; i < lod; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(lod - 1);
		curve.push_back(bezierPoint(u, p0, p1, p2, p3));
	}
	return curve;
}

void addPrimitive(GLenum mode, const std::vector<glm::vec3>& vertices, float pointSize = 1.0f)
{
	if (vertices.empty()) {
		return;
	}

	DrawCommand cmd;
	// "first" retine de unde incepe aceasta primitiva in bufferul mare
	cmd.mode = mode;
	cmd.first = static_cast<GLint>(points.size() / 3);
	cmd.count = static_cast<GLsizei>(vertices.size());
	cmd.pointSize = pointSize;

	for (const glm::vec3& v : vertices) {
		points.push_back(v.x);
		points.push_back(v.y);
		points.push_back(v.z);
	}

	drawCommands.push_back(cmd);
}

void buildExercise3()
{
	// EX3: familie de curbe Coons/Hermite
	// punctele raman fixe, doar directia lui T0 se roteste din 45 in 45 grade
	const float sceneScale = 1.4f;
	const glm::vec3 p0Local(-2.0f * sceneScale, 0.0f, 0.0f);
	const glm::vec3 p1Local(2.0f * sceneScale, 0.0f, 0.0f);
	const float tangentMagnitude = 18.0f * sceneScale;
	const glm::vec3 t1(0.0f, -tangentMagnitude, 0.0f);
	// liniile de directie le desenam mai scurte, sa fie mai clare in imagine
	const float directionLineScale = 0.35f;

	const float anglesDeg[6] = {180.0f, 135.0f, 90.0f, 45.0f, 0.0f, -90.0f};
	const glm::vec3 centers[6] = {
		glm::vec3(-20.0f, 14.0f, 0.0f), glm::vec3(0.0f, 14.0f, 0.0f), glm::vec3(20.0f, 14.0f, 0.0f),
		glm::vec3(-20.0f, -8.0f, 0.0f), glm::vec3(0.0f, -8.0f, 0.0f), glm::vec3(20.0f, -8.0f, 0.0f)
	};

	for (int i = 0; i < 6; ++i) {
		float angleRad = glm::radians(anglesDeg[i]);
		glm::vec3 t0(std::cos(angleRad) * tangentMagnitude, std::sin(angleRad) * tangentMagnitude, 0.0f);

		glm::vec3 p0 = centers[i] + p0Local;
		glm::vec3 p1 = centers[i] + p1Local;

		std::vector<glm::vec3> curve = sampleHermite(p0, p1, t0, t1, LOD_HERMITE);
		// curba propriu-zisa
		addPrimitive(GL_LINE_STRIP, curve);
		// punctele capat
		addPrimitive(GL_POINTS, { p0, p1 }, 10.0f);
		// liniile de directie (vectori tangenti desenati ca segmente)
		addPrimitive(GL_LINES, { p0, p0 + t0 * directionLineScale, p1, p1 + t1 * directionLineScale });
	}
}

void buildExercise1()
{
	// EX1: lant de 3 curbe Coons/Hermite
	// P1 si P2 sunt punctele de imbinare intre segmente
	const std::vector<glm::vec3> P = {
		glm::vec3(-18.0f, -8.0f, 0.0f),
		glm::vec3(-10.0f, -8.0f, 0.0f), // imbinare 1
		glm::vec3(-3.0f, 12.0f, 0.0f),  // imbinare 2
		glm::vec3(16.0f, 15.0f, 0.0f)
	};
	const std::vector<glm::vec3> T = {
		glm::vec3(0.0f, -8.0f, 0.0f),
		glm::vec3(8.0f, 0.0f, 0.0f),
		glm::vec3(8.0f, 6.0f, 0.0f),
		glm::vec3(6.0f, -3.0f, 0.0f)
	};

	for (int i = 0; i < 3; ++i) {
		std::vector<glm::vec3> seg = sampleHermite(P[i], P[i + 1], T[i], T[i + 1], LOD_EX1);
		addPrimitive(GL_LINE_STRIP, seg);
	}

	// capete
	addPrimitive(GL_POINTS, { P.front(), P.back() }, 9.0f);
	// puncte de imbinare (marcate mai mare)
	addPrimitive(GL_POINTS, { P[1], P[2] }, 14.0f);

	std::printf("Ex1 imbinari: J1=(%.1f, %.1f), J2=(%.1f, %.1f)\n", P[1].x, P[1].y, P[2].x, P[2].y);
}

void buildExercise4()
{
	// EX4: curba Bezier cubica + poligonul de control
	const glm::vec3 p0(-12.0f, -5.0f, 0.0f);
	const glm::vec3 p1(-6.0f, 8.0f, 0.0f);
	const glm::vec3 p2(6.0f, 8.0f, 0.0f);
	const glm::vec3 p3(12.0f, -5.0f, 0.0f);

	std::vector<glm::vec3> curve = sampleBezier(p0, p1, p2, p3, LOD_BEZIER);
	// curba
	addPrimitive(GL_LINE_STRIP, curve);
	// poligonul de control
	addPrimitive(GL_LINE_STRIP, { p0, p1, p2, p3 });
	// punctele de control
	addPrimitive(GL_POINTS, { p0, p1, p2, p3 }, 10.0f);
}

void buildExercise5()
{
	// EX5: B-spline cubica compusa din 3 segmente
	// U = [0,0,0,0, 1,2,3, 3,3,3] => segmente pe [0,1], [1,2], [2,3]
	std::vector<glm::vec3> controlPoints = {
		glm::vec3(-18.0f, -8.0f, 0.0f),
		glm::vec3(-14.0f, -12.0f, 0.0f),
		glm::vec3(-9.0f, -8.0f, 0.0f),
		glm::vec3(-3.0f, 12.0f, 0.0f),
		glm::vec3(10.0f, 16.0f, 0.0f),
		glm::vec3(16.0f, 15.0f, 0.0f)
	};

	const int order = 4;
	std::vector<float> knots = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 3.0f, 3.0f, 3.0f };

	// desenam explicit cele 3 segmente
	addPrimitive(GL_LINE_STRIP, sampleBSplineSegment(controlPoints, order, knots, 0.0f, 1.0f, LOD_BSPLINE / 3));
	addPrimitive(GL_LINE_STRIP, sampleBSplineSegment(controlPoints, order, knots, 1.0f, 2.0f, LOD_BSPLINE / 3));
	addPrimitive(GL_LINE_STRIP, sampleBSplineSegment(controlPoints, order, knots, 2.0f, 3.0f, LOD_BSPLINE / 3));

	// puncte de imbinare intre segmente la t=1 si t=2
	glm::vec3 j1 = evaluateBSplinePoint(controlPoints, order, knots, 1.0f);
	glm::vec3 j2 = evaluateBSplinePoint(controlPoints, order, knots, 2.0f);
	addPrimitive(GL_POINTS, { j1, j2 }, 14.0f);

	// optional: poligon de control, sa vezi influenta punctelor
	addPrimitive(GL_LINE_STRIP, controlPoints);
	addPrimitive(GL_POINTS, controlPoints, 7.0f);

	std::printf("Ex5 imbinari B-spline: J1=(%.2f, %.2f), J2=(%.2f, %.2f)\n", j1.x, j1.y, j2.x, j2.y);
}

void createPointsVector()
{
	// "scene builder": aici alegem ce exercitiu desenam
	points.clear();
	drawCommands.clear();

	if (currentExercise == 1) buildExercise1();
	if (currentExercise == 3) buildExercise3();
	if (currentExercise == 4) buildExercise4();
	if (currentExercise == 5) buildExercise5();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_programme);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));

	glBindVertexArray(vao);
	// Desenam pe rand tot ce am pus in drawCommands
	for (const DrawCommand& cmd : drawCommands) {
		if (cmd.mode == GL_POINTS) {
			glPointSize(cmd.pointSize);
		}
		glDrawArrays(cmd.mode, cmd.first, cmd.count);
	}

	glutSwapBuffers();
}

void uploadSceneToGpu()
{
	createPointsVector();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);
}

void keyboard(unsigned char key, int x, int y)
{
	(void)x; (void)y;

	if (key == '1' || key == '3' || key == '4' || key == '5') {
		currentExercise = key - '0';
		uploadSceneToGpu();
		updateProjection(gWindowW, gWindowH);
		std::printf("Afisare Ex%d\n", currentExercise);
		glutPostRedisplay();
	}
}

void init()
{
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::printf("Renderer: %s\n", renderer);
	std::printf("OpenGL version supported %s\n", version);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glewInit();
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	createPointsVector();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// trimitem punctele in GPU o singura data (date statice)
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	// atributul 0 din shader primeste vec3 (x,y,z) consecutiv
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	std::string vstext = textFileRead("vertex.vert");
	std::string fstext = textFileRead("fragment.frag");

	GLuint vs = compileShader(GL_VERTEX_SHADER, vstext.c_str());
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, fstext.c_str());

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	GLint linkStatus = GL_FALSE;
	glGetProgramiv(shader_programme, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		GLint logLen = 0;
		glGetProgramiv(shader_programme, GL_INFO_LOG_LENGTH, &logLen);
		std::vector<char> log(logLen + 1);
		glGetProgramInfoLog(shader_programme, logLen, NULL, log.data());
		std::printf("Program link error: %s\n", log.data());
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	gWindowW = w;
	gWindowH = h;
	updateProjection(w, h);
}

void updateProjection(int w, int h)
{
	float aspect = (h == 0) ? 1.0f : static_cast<float>(w) / static_cast<float>(h);
	float halfHeight = 55.0f;

	// Ex1/4/5 mai mari pe ecran
	if (currentExercise == 1) halfHeight = 18.0f;
	if (currentExercise == 4) halfHeight = 16.0f;
	if (currentExercise == 5) halfHeight = 18.0f;
	if (currentExercise == 3) halfHeight = 55.0f;

	float halfWidth = halfHeight * aspect;

	// proiectie ortografica: pastreaza figurile "tehnice" fara perspectiva
	projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);
	viewMatrix = glm::mat4(1.0f);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("SPG Lab13 - Ex1 + Ex3 + Ex4 + Ex5");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	std::printf("Taste: 1 = Ex1, 3 = Ex3, 4 = Ex4, 5 = Ex5\n");
	glutMainLoop();

	return 0;
}
