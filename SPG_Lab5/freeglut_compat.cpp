// GLFW-based implementation of the freeglut API subset used in SPG_Lab5.
// Extends Lab3 compat with: glutReshapeFunc, glutKeyboardFunc, glutTimerFunc,
// glutPostRedisplay, GLUT_DOUBLE support, and glutSwapBuffers.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

static GLFWwindow* g_window        = nullptr;
static void (*g_display_func)()    = nullptr;
static void (*g_reshape_func)(int,int) = nullptr;
static void (*g_keyboard_func)(unsigned char, int, int) = nullptr;
static void (*g_timer_func)(int)   = nullptr;
static int  g_timer_value          = 0;
static double g_timer_ms           = 0.0;   // milliseconds until timer fires
static double g_timer_target       = -1.0;  // glfwGetTime() target
static int  g_win_x = 200, g_win_y = 200;
static int  g_win_w = 700, g_win_h = 700;
static bool g_redisplay            = false;

extern "C" {

void glutInit(int* argc, char** argv)
{
    (void)argc; (void)argv;
    if (!glfwInit()) {
        fprintf(stderr, "freeglut_compat: glfwInit() failed\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void glutInitDisplayMode(unsigned int mode)
{
    (void)mode;
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
    glewExperimental = GL_TRUE;

    // Forward resize events
    glfwSetFramebufferSizeCallback(g_window, [](GLFWwindow*, int w, int h) {
        if (g_reshape_func) g_reshape_func(w, h);
    });

    // Forward key events (printable ASCII only, like freeglut sends)
    glfwSetCharCallback(g_window, [](GLFWwindow*, unsigned int codepoint) {
        if (g_keyboard_func)
            g_keyboard_func((unsigned char)codepoint, 0, 0);
    });

    return 1;
}

void glutDisplayFunc(void (*func)(void))    { g_display_func  = func; }
void glutReshapeFunc(void (*func)(int,int)) { g_reshape_func  = func; }
void glutKeyboardFunc(void (*func)(unsigned char, int, int)) { g_keyboard_func = func; }

void glutTimerFunc(unsigned int millis, void (*func)(int), int value)
{
    g_timer_func   = func;
    g_timer_value  = value;
    g_timer_ms     = (double)millis;
    g_timer_target = glfwGetTime() + millis / 1000.0;
}

void glutPostRedisplay() { g_redisplay = true; }

void glutSwapBuffers()
{
    if (g_window) glfwSwapBuffers(g_window);
}

void glutMainLoop(void)
{
    // Fire an initial reshape for the created window size
    if (g_reshape_func) g_reshape_func(g_win_w, g_win_h);

    while (!glfwWindowShouldClose(g_window)) {
        if (g_display_func) {
            g_display_func();
            g_redisplay = false;
            // GLFW has no true single-buffer mode; always swap so that
            // glFlush()-based display functions appear on screen.
            glfwSwapBuffers(g_window);
        }

        // Timer check
        if (g_timer_func && g_timer_target >= 0.0 && glfwGetTime() >= g_timer_target) {
            void (*cb)(int) = g_timer_func;
            int  val        = g_timer_value;
            g_timer_func    = nullptr;
            g_timer_target  = -1.0;
            cb(val);
        }

        glfwPollEvents();
    }
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

} // extern "C"
