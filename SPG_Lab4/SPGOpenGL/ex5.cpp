#include "lab4_common.h"

#include <chrono>
#include <cmath>
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

static Mesh g_square;
static Mesh g_sphere;

static auto g_start_time = std::chrono::steady_clock::now();

// Cronometru de scena folosit la animatia sferei.
static float elapsedSeconds()
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - g_start_time;
    return elapsed.count();
}

// Set de uniformi comun pentru fiecare obiect desenat.
static void setUniforms(const glm::mat4& model,
    const glm::vec3& color,
    int useShaderTransform,
    const glm::vec2& offset,
    float pulse)
{
    if (u_model >= 0) glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    // Culoare per-obiect transmisa prin uniformul `color`.
    if (u_color >= 0) glUniform3fv(u_color, 1, glm::value_ptr(color));

    // Ex5 nu foloseste transformarile din shader, dar pastram aceeasi interfata
    // ca in celelalte ex-uri pentru reutilizarea shaderului comun.
    if (u_useShaderTransform >= 0) glUniform1i(u_useShaderTransform, useShaderTransform);
    if (u_offset >= 0) glUniform2fv(u_offset, 1, glm::value_ptr(offset));
    if (u_pulse >= 0) glUniform1f(u_pulse, pulse);
}

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    float t = elapsedSeconds();
    float radius = 0.55f;

    // Formula ceruta pentru efectul de bouncing:
    // y = radius + fabs(3 * sin(time))
    float y = radius + std::fabs(3.0f * std::sin(t));

    // Scala globala pentru a aduce coordonatele scenei in NDC.
    glm::mat4 worldToNDC = customScale(0.22f, 0.22f, 0.22f);

    // "Podea" pe care ricoseaza sfera.
    glm::mat4 floorModel = worldToNDC
        * customTranslation(2.6f, -2.0f, 0.0f)
        * customScale(2.0f, 0.08f, 1.0f);
    setUniforms(floorModel, glm::vec3(0.1f, 0.1f, 0.1f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_square);

    glm::mat4 squareModel = worldToNDC
        * customTranslation(-2.4f, -1.4f, 0.0f)
        * customScale(1.5f, 1.5f, 1.0f);
    setUniforms(squareModel, glm::vec3(0.0f, 0.55f, 0.2f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_square);

    // Sfera animata: translatie pe Y conform ecuatiei + rotatie proprie.
    glm::mat4 sphereModel = worldToNDC
        * customTranslation(2.6f, y - 2.0f, 0.0f)
        * glm::rotate(t, glm::vec3(0.0f, 1.0f, 0.0f))
        * customScale(radius, radius, radius);

    // Wireframe pentru a vedea clar miscarea suprafetei sferei.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    setUniforms(sphereModel, glm::vec3(0.95f, 0.2f, 0.2f), 0, glm::vec2(0.0f), 0.0f);
    drawMesh(g_sphere);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glutSwapBuffers();
    glutPostRedisplay();
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

    // Cache pentru locatiile uniformelor folosite la fiecare obiect/frame.
    u_model = glGetUniformLocation(shader_programme, "modelMatrix");
    u_color = glGetUniformLocation(shader_programme, "color");
    u_offset = glGetUniformLocation(shader_programme, "uOffset");
    u_pulse = glGetUniformLocation(shader_programme, "uPulse");
    u_useShaderTransform = glGetUniformLocation(shader_programme, "useShaderTransform");

    std::vector<float> squareVertices = {
        -0.5f, 0.5f, 0.0f,
         0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
    std::vector<unsigned int> squareIndices = {
        0, 2, 1,
        1, 2, 3
    };
    setupMesh(g_square, squareVertices, squareIndices);

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;

    // Generam procedural sfera (stacks/slices), apoi o trimitem in GPU.
    generateSphere(1.0f, 24, 32, sphereVertices, sphereIndices);
    setupMesh(g_sphere, sphereVertices, sphereIndices);
}

int main(int argc, char** argv)
{
    if (argc > 0) {
        setAssetDirFromArgv0(argv[0]);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(900, 700);
    glutCreateWindow("SPG Lab4 - Ex5");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
