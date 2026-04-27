#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <vector>

// Cerc format din triunghiuri cu centrul comun (triangle fan manual)
// Fiecare triunghi: centru (0,0,0), punct[i], punct[i+1]

static const int   N      = 16;       // numarul de triunghiuri (segmente)
static const float RADIUS = 0.7f;
static const float PI     = 3.14159265358979f;

GLuint shader_programme, vao;
static int vertex_count = 0;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glutSwapBuffers();
}

void init()
{
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glClearColor(1, 1, 1, 0);
    glewInit();

    // Construim varfurile: 3 varfuri per triunghi * N triunghiuri
    std::vector<float> points;
    for (int i = 0; i < N; ++i) {
        float a0 = 2.0f * PI * i       / N;
        float a1 = 2.0f * PI * (i + 1) / N;

        // centru
        points.push_back(0.0f);
        points.push_back(0.0f);
        points.push_back(0.0f);
        // punct pe cerc la unghi a0
        points.push_back(RADIUS * cosf(a0));
        points.push_back(RADIUS * sinf(a0));
        points.push_back(0.0f);
        // punct pe cerc la unghi a1
        points.push_back(RADIUS * cosf(a1));
        points.push_back(RADIUS * sinf(a1));
        points.push_back(0.0f);
    }
    vertex_count = N * 3;

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const char *vertex_shader =
        "#version 400\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

    const char *fragment_shader =
        "#version 400\n"
        "out vec4 frag_colour;"
        "void main() {"
        "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
        "}";

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

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(512, 512);
    glutCreateWindow("SPG - Ex3: Cerc din triunghiuri (VBO)");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
