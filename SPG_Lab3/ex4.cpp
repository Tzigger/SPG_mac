#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <vector>

// Cerc format din triunghiuri folosind Index Buffer Object (IBO / EBO)
// Varfuri unice: centru (index 0) + N puncte pe cerc (indecsi 1..N)
// Indici: pentru fiecare sector i -> { 0, i+1, (i+1)%N + 1 }

static const int   N      = 16;
static const float RADIUS = 0.7f;
static const float PI     = 3.14159265358979f;

GLuint shader_programme, vao;
static int index_count = 0;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
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

    std::vector<float> points;
    // centru
    points.push_back(0.0f);
    points.push_back(0.0f);
    points.push_back(0.0f);
    // puncte pe cerc
    for (int i = 0; i < N; ++i) {
        float a = 2.0f * PI * i / N;
        points.push_back(RADIUS * cosf(a));
        points.push_back(RADIUS * sinf(a));
        points.push_back(0.0f);
    }

    std::vector<unsigned int> indices;
    for (int i = 0; i < N; ++i) {
        indices.push_back(0);           // centru
        indices.push_back(i + 1);       // punct curent pe cerc
        indices.push_back((i + 1) % N + 1); // urmatorul punct 
    }
    index_count = (int)indices.size();

    // ---------- VBO ----------
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    // ---------- IBO ----------
    GLuint ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // ---------- VAO ----------
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // IBO-ul se leaga in VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

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
    glutCreateWindow("SPG - Ex4: Cerc din triunghiuri (IBO)");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
