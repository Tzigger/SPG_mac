#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#define PI     glm::pi<float>()
#define DEG5   (5.0f * PI / 180.0f)

// ── Vertex data ──────────────────────────────────────────────────────────────
// Cub unitate cu un colt in origine: varfuri in [0,1]^3
// 12 muchii = 24 puncte (GL_LINES)
static float points[] = {
    // Fata spate (z = 0)
    0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f,
    // Fata fata  (z = 1)
    0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
    // Muchii laterale
    0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 1.0f,
    // Axa de rotatie: diagonala principala
    0.0f, 0.0f, 0.0f,   1.0f,  1.0f,  1.0f,  // offset 24, 2 pts
    // Axele de coordonate WCS
     0.0f,  0.0f,  0.0f,   16.0f,  0.0f,  0.0f,  // axa X, offset 26
     0.0f,  0.0f,  0.0f,    0.0f,  8.0f,  0.0f,  // axa Y, offset 28
     0.0f,  0.0f,  0.0f,    0.0f,  0.0f, 24.0f,  // axa Z, offset 30
};

static GLuint shader_programme, vao;
static glm::mat4 projectionMatrix, viewMatrix;

// Unghiul de rotatie acumulat (radiani)
static float rotAngle = 0.0f;

// Axa de rotatie: diagonala (0,0,0)→(1,1,1) normalizata
static const glm::vec3 DIAG_AXIS = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

// ── Citire fisier shader ─────────────────────────────────────────────────────
std::string textFileRead(const char *fn)
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

// ── Display ──────────────────────────────────────────────────────────────────
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glBindVertexArray(vao);

    GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
    GLuint colorID  = glGetUniformLocation(shader_programme, "color");

    /*
     * Rotatia in jurul diagonalei principale:
     * Diagonala trece prin origine si prin (1,1,1).
     * Centrul cubului este la (0.5, 0.5, 0.5).
     * Pasii:
     *   1) translatie la origine (centrul cubului in origine)
     *   2) rotatie in jurul axei (1,1,1)/sqrt(3)
     *   3) translatie inapoi
     * Sau mai simplu, rotim in jurul diagonalei care trece chiar prin origine,
     * deoarece coltul (0,0,0) este fix pe diagonala.
     * Centrul de rotatie ales: originea (coltul (0,0,0)).
     * Rotim direct in jurul diagonalei normalizate care pleaca din origine.
     */
    glm::mat4 modelMatrix = glm::rotate(rotAngle, DIAG_AXIS);

    // Cub – albastru, se roteste cu modelMatrix
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glUniform3f(colorID, 0.1f, 0.3f, 0.9f);
    glDrawArrays(GL_LINES, 0, 24);

    // Axa diagonala – rosie, fixa
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * identity));
    glUniform3f(colorID, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 24, 2);

    // Axele WCS – negru, fixe
    glUniform3f(colorID, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 26, 6);

    glFlush();
}

// ── Keyboard ─────────────────────────────────────────────────────────────────
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
    case 's':   // rotatie stanga
        rotAngle += DEG5;
        break;
    case 'd':   // rotatie dreapta
        rotAngle -= DEG5;
        break;
    case 27:    // ESC – iesire
        exit(0);
    default:
        break;
    }
    glutPostRedisplay();
}

// ── Init ─────────────────────────────────────────────────────────────────────
void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glClearColor(1, 1, 1, 0);
    glEnable(GL_DEPTH_TEST);
    glewInit();

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");
    const char* vertex_shader   = vstext.c_str();
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
}

// ── Reshape ───────────────────────────────────────────────────────────────────
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    /*
     * Proiectie ortogonala.
     * Alegem un volum de vizualizare simetric care sa cuprinda confortabil
     * cubul unitate (la orice rotatie diagonala el ramane in [-1, 2]).
     */
    float aspect = (h > 0) ? (float)w / h : 1.0f;
    float halfH  = 2.0f;
    float halfW  = halfH * aspect;
    projectionMatrix = glm::ortho(-halfW, halfW, -halfH, halfH, -10.0f, 10.0f);

    /*
     * Observatorul pozitionat deasupra si in fata scenei, privind spre centrul
     * cubului (0.5, 0.5, 0.5).
     */
    viewMatrix = glm::lookAt(
        glm::vec3(2.5f, 2.5f, 5.0f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(700, 700);
    glutCreateWindow("SPG Lab5 - Problema 3: Rotatie cub pe diagonala (s/d)");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
