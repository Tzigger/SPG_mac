#include "lab4_common.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/freeglut.h>

static GLuint shader_programme = 0;
static GLuint vao = 0;

static GLint u_scale = -1;
static GLint u_translation = -1;
static GLint u_rotation = -1;


static float g_scale_value = 0.6f;
static float g_translation_x = -0.5f;
static float g_translation_y = 0.0f;
static float g_translation_z = 0.0f;
static float g_rotation_angle = kPi * 0.8f;

// Varfurile triunghiului
static float points[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
   -0.5f, -0.5f,  0.0f
};

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);

    if (u_scale >= 0) {
        glUniform1f(u_scale, g_scale_value);
    }
    if (u_translation >= 0) {
        glUniform3f(u_translation, g_translation_x, g_translation_y, g_translation_z);
    }
    if (u_rotation >= 0) {
        glUniform1f(u_rotation, g_rotation_angle);
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glFlush();
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
    glewInit();

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    shader_programme = buildShaderProgram("vertex_ex4.vert", "fragment_ex4.frag");
    if (shader_programme == 0) {
        exit(1);
    }

    u_scale = glGetUniformLocation(shader_programme, "scaleFactor");
    u_translation = glGetUniformLocation(shader_programme, "translationVec");
    u_rotation = glGetUniformLocation(shader_programme, "rotationAngle");

    printf("Uniform locations: scaleFactor=%d translationVec=%d rotationAngle=%d\n",
           u_scale, u_translation, u_rotation);
}

int main(int argc, char** argv)
{
    if (argc > 0) {
        setAssetDirFromArgv0(argv[0]);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(512, 512);
    glutCreateWindow("SPG Lab4 - Ex4");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
