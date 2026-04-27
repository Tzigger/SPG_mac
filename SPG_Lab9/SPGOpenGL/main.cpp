#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern "C" {
    void glutInit(int*, char**);
    void glutInitDisplayMode(unsigned int);
    void glutInitWindowPosition(int, int);
    void glutInitWindowSize(int, int);
    int  glutCreateWindow(const char*);
    void glutDisplayFunc(void (*)());
    void glutReshapeFunc(void (*)(int, int));
    void glutKeyboardFunc(void (*)(unsigned char, int, int));
    void glutMainLoop();
    void glutPostRedisplay();
    void glutSwapBuffers();
}
#define GLUT_RGB    0
#define GLUT_DOUBLE 2
#define GLUT_DEPTH  16

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

#include "objloader.hpp"

#define PI glm::pi<float>()

GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

GLuint vaoObj, vboObj;

// Parametrii camerei (observatorului)
glm::vec3 cameraPos(10, 12, 30);
glm::vec3 cameraFront = glm::normalize(glm::vec3(-10, -12, -30)); // directia de privire initiala spre origine
glm::vec3 cameraUp(0, 1, 0);
float cameraSpeed = 0.5f;

// Sursa 1 de lumina: fixa in scena
// Sursa 2 de lumina: se va actualiza cu pozitia camerei in display()
glm::vec3 lightPos1(0, 200, 100);
glm::vec3 lightPos2;

// Parametrii de transformare controlati din tastatura
float axisRotAngle = 0.0f; // unghiul de rotatie al modelelor (Q/E)
float scaleFactor  = 0.01f; // scara globala a modelelor (+/-)

// Pozitiile, rotatiile extra si scalele pentru cele 4 instante ale obiectului
glm::vec3 instancePos[]   = { {0,0,0}, {6,0,-4}, {-6,0,-3}, {1,0,-10} };
float     instanceRot[]   = { 0.0f, PI/3.0f, -PI/4.0f, PI };
float     instanceScale[]  = { 1.0f, 0.65f, 0.8f, 1.2f };
int       numInstances     = 4;

void printShaderInfoLog(GLuint obj)
{
    int len = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        char* log = (char*)malloc(len);
        glGetShaderInfoLog(obj, len, NULL, log);
        printf("%s\n", log);
        free(log);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int len = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        char* log = (char*)malloc(len);
        glGetProgramInfoLog(obj, len, NULL, log);
        printf("%s\n", log);
        free(log);
    }
}

// Citeste un fisier text (folosit pentru a incarca sursele shaderelor)
std::string textFileRead(const char *fn)
{
    std::ifstream f(fn);
    std::string text, line;
    while (f.good()) {
        std::getline(f, line);
        text += line + "\n";
    }
    return text;
}

void init()
{
    glewExperimental = GL_TRUE;
    glewInit();

    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    loadOBJ("obj/scrat.obj", vertices, uvs, normals);
    printf("Vertexuri incarcate: %zu\n", vertices.size());

    glEnable(GL_DEPTH_TEST);
    glClearColor(1,1,1,1);//glClearColor(0.12f, 0.14f, 0.18f, 1.0f);

    // Cream VAO si VBO
    // Layout VBO: [toate pozitiile | toate normalele]
    glGenVertexArrays(1, &vaoObj);
    glBindVertexArray(vaoObj);

    glGenBuffers(1, &vboObj);
    glBindBuffer(GL_ARRAY_BUFFER, vboObj);

    size_t vSize = vertices.size() * sizeof(glm::vec3);
    size_t nSize = normals.size()  * sizeof(glm::vec3);
    glBufferData(GL_ARRAY_BUFFER, vSize + nSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,     vSize, vertices.data()); // pozitii
    glBufferSubData(GL_ARRAY_BUFFER, vSize, nSize, normals.data());  // normale

    // location=0: pozitii, location=1: normale
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)vSize);

    std::string vsText = textFileRead("vertex.vert");
    std::string fsText = textFileRead("fragment.frag");
    const char* vs_src = vsText.c_str();
    const char* fs_src = fsText.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    printShaderInfoLog(vs);
    printShaderInfoLog(fs);
    printProgramInfoLog(shader_programme);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);
    glBindVertexArray(vaoObj);

    // Sursa 2 de lumina se deplaseaza cu observatorul
    lightPos2 = cameraPos;

    // Trimitem pozitiile luminilor si ale camerei la fragment shader
    glUniform3fv(glGetUniformLocation(shader_programme, "lightPos1"), 1, glm::value_ptr(lightPos1));
    glUniform3fv(glGetUniformLocation(shader_programme, "lightPos2"), 1, glm::value_ptr(lightPos2));
    glUniform3fv(glGetUniformLocation(shader_programme, "viewPos"),   1, glm::value_ptr(cameraPos));

    for (int i = 0; i < numInstances; i++) {
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, instancePos[i]);
        modelMatrix = glm::rotate(modelMatrix, axisRotAngle + instanceRot[i], glm::vec3(0, 1, 0));
        float s = scaleFactor * instanceScale[i];
        modelMatrix = glm::scale(modelMatrix, glm::vec3(s, s, s));

        // MVP = Proiectie * Vizualizare * Modelare
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        // Matricea de normale = Transpose(Inverse(M)) - necesara pentru normalizarea corecta
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

        glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelMatrix"),              1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"),             1, GL_FALSE, glm::value_ptr(normalMatrix));

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    }

    glutSwapBuffers();
}


void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 4.0f, (float)w / h, 0.1f, 1000.0f);
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void keyboard(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    switch (key) {
    case 'q': case 'Q': axisRotAngle += 0.1f; break;
    case 'e': case 'E': axisRotAngle -= 0.1f; break;
    case '+': scaleFactor += 0.001f; break;
    case '-': if (scaleFactor > 0.001f) scaleFactor -= 0.001f; break;
    case 'w': case 'W':
        cameraPos += cameraSpeed * cameraFront;
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        break;
    case 's': case 'S':
        cameraPos -= cameraSpeed * cameraFront;
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        break;
    case 'd': case 'D':
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        break;
    case 'a': case 'A':
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(900, 700);
    glutCreateWindow("SPG Lab 9");
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    printf("Q/E=rotire  +/-=scala  WASD=camera  ESC=exit\n");
    glutMainLoop();
    return 0;
}
