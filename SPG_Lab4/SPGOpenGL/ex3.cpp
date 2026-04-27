#include "lab4_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/freeglut.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

static GLuint shader_programme = 0;
static GLint u_model = -1;
static GLint u_color = -1;
static GLint u_offset = -1;
static GLint u_pulse = -1;
static GLint u_useShaderTransform = -1;

static Mesh g_triangle;
static int g_order_index = 0;

static const char* g_order_names[] = {
    "T * R * S",
    "R * T * S",
    "S * R * T"
};

// Trimite tot setul de uniformi inainte de draw call.
// In ex3 folosim in principal modelMatrix + color.
static void setUniforms(const glm::mat4& model,
    const glm::vec3& color,
    int useShaderTransform,
    const glm::vec2& offset,
    float pulse)
{
    if (u_model >= 0) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    // Culoare per-obiect: CPU -> glUniform3fv -> fragment shader (uniform vec3 color).
    if (u_color >= 0) glUniform3fv(u_color, 1, glm::value_ptr(color));

    // Uniformii de shader transform raman disponibili pentru acelasi shader comun.
    if (u_useShaderTransform >= 0) glUniform1i(u_useShaderTransform, useShaderTransform);
    if (u_offset >= 0) glUniform2fv(u_offset, 1, glm::value_ptr(offset));
    if (u_pulse >= 0) glUniform1f(u_pulse, pulse);
}

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    // Construim separat T, R, S (matrici individuale), apoi le compunem.
    glm::mat4 T = customTranslation(0.25f, -0.25f, 0.0f);
    glm::mat4 R = customRotationZ(kPi * 0.35f);
    glm::mat4 S = customScale(0.6f, 1.25f, 1.0f);

    glm::mat4 model(1.0f);
    // Schimbarea ordinii arata direct ca T/R/S NU comuta.
    if (g_order_index == 0) {
        model = T * R * S;
    } else if (g_order_index == 1) {
        model = R * T * S;
    } else {
        model = S * R * T;
    }

    setUniforms(model, glm::vec3(0.0f, 0.65f, 0.3f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_triangle);

    glutSwapBuffers();
}

static void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    // Tasta O itereaza ordinea de inmultire a matricilor in ex3.
    if (key == 'o' || key == 'O') {
        g_order_index = (g_order_index + 1) % 3;
        printf("Order switched to: %s\n", g_order_names[g_order_index]);
        glutPostRedisplay();
    }

    if (key == 27) {
        exit(0);
    }
}

static void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
    printf("Press O to change T/R/S order\n");

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    glewInit();

    shader_programme = buildShaderProgram("vertex.vert", "fragment.frag");
    if (shader_programme == 0) {
        exit(1);
    }

    // Cautam o singura data locatiile uniformelor, apoi doar actualizam valori.
    u_model = glGetUniformLocation(shader_programme, "modelMatrix");
    u_color = glGetUniformLocation(shader_programme, "color");
    u_offset = glGetUniformLocation(shader_programme, "uOffset");
    u_pulse = glGetUniformLocation(shader_programme, "uPulse");
    u_useShaderTransform = glGetUniformLocation(shader_programme, "useShaderTransform");

    std::vector<float> triangleVertices = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };
    setupMesh(g_triangle, triangleVertices, {});
}

int main(int argc, char** argv)
{
    if (argc > 0) {
        setAssetDirFromArgv0(argv[0]);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(700, 700);
    glutCreateWindow("SPG Lab4 - Ex3");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
