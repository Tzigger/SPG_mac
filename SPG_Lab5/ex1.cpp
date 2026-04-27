#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#define PI glm::pi<float>()

GLuint shader_programme, vao;
glm::mat4 projectionMatrix, viewMatrix;

// 12 muchii cub = 24 puncte (offset 0..23) + 3 axe WCS = 6 puncte (offset 24..29)
float points[] = {
    // Fata spate (z = -0.5) – 4 muchii
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    // Fata fata (z = +0.5) – 4 muchii
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    // Muchii laterale – 4 muchii
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
    // Axele de coordonate
     0.0f,  0.0f,  0.0f,
    16.0f,  0.0f,  0.0f,  // axa X
     0.0f,  0.0f,  0.0f,
     0.0f,  8.0f,  0.0f,  // axa Y
     0.0f,  0.0f,  0.0f,
     0.0f,  0.0f, 24.0f,  // axa Z
};

float xv = 2, yv = 2, zv = 30; // originea sistemului de observare
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

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);

    glm::mat4 modelMatrix(1.0f); // matrice identitate
    glBindVertexArray(vao);

    GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
    GLuint colorID  = glGetUniformLocation(shader_programme, "color");

    glUniform3f(colorID, 0.0f, 0.0f, 0.0f);

    // ── Axe coordonate ─────────────────────────────────────────────────────
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 24, 6); // toate cele 3 axe dintr-un apel

    // ── Cub 1: identitate – centrat in origine ─────────────────────────────
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 0, 24);

    // ── Cub 2: S(2,2,2), T(0,-8,0) ────────────────────────────────────────
    // Mi2 = T(0,-8,0) * S(2,2,2)
    modelMatrix = glm::translate(glm::vec3(0.0f, -8.0f, 0.0f))
                * glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 0, 24);

    // ── Cub 3: S(2,1,2), Rz(pi/4), T(8,0,0) ──────────────────────────────
    // Mi3 = T(8,0,0) * Rz(pi/4) * S(2,1,2)
    modelMatrix = glm::translate(glm::vec3(8.0f, 0.0f, 0.0f))
                * glm::rotate(PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f))
                * glm::scale(glm::vec3(2.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 0, 24);

    // ── Cub 4: S(2,1,2), Rz(-pi/4), T(-8,0,0) ────────────────────────────
    // Mi4 = T(-8,0,0) * Rz(-pi/4) * S(2,1,2)
    modelMatrix = glm::translate(glm::vec3(-8.0f, 0.0f, 0.0f))
                * glm::rotate(-PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f))
                * glm::scale(glm::vec3(2.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 0, 24);

    glFlush();
}

void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
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

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 3, (float)w / h, 0.1f, 100.0f);

    viewMatrix = glm::lookAt(
        glm::vec3(xv, yv, zv),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(700, 700);
    glutCreateWindow("SPG");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
