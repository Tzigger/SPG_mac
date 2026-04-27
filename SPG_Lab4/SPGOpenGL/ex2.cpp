#include "lab4_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/freeglut.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

static GLuint shader_programme = 0;
static GLint u_model = -1;
static GLint u_color = -1;
static GLint u_offset = -1;
static GLint u_pulse = -1;
static GLint u_useShaderTransform = -1;

static Mesh g_triangle;


static void setUniforms(const glm::mat4& model,
    const glm::vec3& color,
    int useShaderTransform,
    const glm::vec2& offset,
    float pulse)
{
    if (u_model >= 0) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    // Culoarea este pasata din CPU catre fragment shader prin uniformul `color`.
    if (u_color >= 0) glUniform3fv(u_color, 1, glm::value_ptr(color));

}

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    // Primul triunghi: folosim matricea de transformare construita manual.
    glm::mat4 leftCustom = customTranslation(-0.5f, 0.0f, 0.0f);
    setUniforms(leftCustom, glm::vec3(0.0f, 0.45f, 0.9f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_triangle);

    // Al doilea triunghi: translatie la dreapta + rotatie 90 de grade.
    glm::mat4 rightGLM = glm::translate(glm::vec3(0.5f, 0.0f, 0.0f))
        * glm::rotate(kPi * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
    setUniforms(rightGLM, glm::vec3(0.9f, 0.3f, 0.1f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_triangle);

    glutSwapBuffers();
}

static void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
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

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    glewInit();

    shader_programme = buildShaderProgram("vertex.vert", "fragment.frag");
    if (shader_programme == 0) {
        exit(1);
    }

    // Legam uniformele o singura data dupa link; apoi schimbam valorile
    // pe fiecare draw call in functie de obiect.
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
    glutCreateWindow("SPG Lab4 - Ex2");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
