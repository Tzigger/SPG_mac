#pragma once
// Compatibility header: replaces freeglut with a GLFW-based implementation
// so the code compiles and runs natively on macOS without XQuartz.

#define GLUT_RGB    0x0000
#define GLUT_RGBA   0x0000
#define GLUT_DOUBLE 0x0002
#define GLUT_DEPTH  0x0010

#ifdef __cplusplus
extern "C" {
#endif

void glutInit(int* argc, char** argv);
void glutInitDisplayMode(unsigned int mode);
void glutInitWindowPosition(int x, int y);
void glutInitWindowSize(int w, int h);
int  glutCreateWindow(const char* title);
void glutDisplayFunc(void (*func)(void));
void glutMainLoop(void);
void glutSwapBuffers(void);

#ifdef __cplusplus
}
#endif
