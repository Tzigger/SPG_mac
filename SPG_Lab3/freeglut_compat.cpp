// GLFW-based implementation of the freeglut API subset used in SPG_Lab3.
// This lets main.cpp run natively on macOS without XQuartz.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

static GLFWwindow* g_window      = nullptr;
static void (*g_display_func)()  = nullptr;
static int g_win_x = 100, g_win_y = 100;
static int g_win_w = 640, g_win_h = 480;

extern "C" {

void glutInit(int* argc, char** argv)
{
    (void)argc; (void)argv;
    if (!glfwInit()) {
        fprintf(stderr, "freeglut_compat: glfwInit() failed\n");
        exit(1);
    }
    // Request OpenGL 4.0 Core Profile (matches the #version 400 shaders)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void glutInitDisplayMode(unsigned int mode)
{
    (void)mode; // GLFW handles double-buffering by default
}

void glutInitWindowPosition(int x, int y)
{
    g_win_x = x;
    g_win_y = y;
}

void glutInitWindowSize(int w, int h)
{
    g_win_w = w;
    g_win_h = h;
}

int glutCreateWindow(const char* title)
{
    g_window = glfwCreateWindow(g_win_w, g_win_h, title, nullptr, nullptr);
    if (!g_window) {
        fprintf(stderr, "freeglut_compat: glfwCreateWindow() failed\n");
        glfwTerminate();
        return 0;
    }
    glfwSetWindowPos(g_window, g_win_x, g_win_y);
    glfwMakeContextCurrent(g_window);

    // GLEW needs this for core profiles
    glewExperimental = GL_TRUE;

    return 1;
}

void glutDisplayFunc(void (*func)(void))
{
    g_display_func = func;
}

void glutMainLoop(void)
{
    while (!glfwWindowShouldClose(g_window)) {
        if (g_display_func)
            g_display_func();
        glfwPollEvents();
    }
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

void glutSwapBuffers(void)
{
    glfwSwapBuffers(g_window);
}

} // extern "C"
