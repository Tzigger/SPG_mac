#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include "spheremesh.h"
#include <stack>

#define PI glm::pi<float>()

GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;
std::stack<glm::mat4> modelStack;

GLuint vboAxes, vaoAxes;
GLuint vboSphere, vaoSphere;
GLuint eboSphere;

// Variabila pt a pastra exercitiul curent activat din taste
int currentExercise = 1;

float axes[] = {
    // Axele de coordonate X, Y, Z pentru desenarea ulterioara
    0.0f, 0.0f, 0.0f,
    16.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 8.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 16.0f
};

// Generam sfera si scoatem numarul de bytes pentru draw elements
SphereMesh sphere;
int sphereElementCount = (GLsizei)sphere.triangles.size() * sizeof(glm::ivec3);

// Valorile unde e plasata camera (vezi functia reshape > viewMatrix)
float xv = 10, yv = 12, zv = 30;

// Citire fisier fragment/vertex si eliminare trailing "\r" ca sa parsam normal shader-ul
std::string textFileRead(const char* fn)
{
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        // Scoatem caracterul Windows "\r" daca e prezent
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        filetext.append(line + "\n");
    }
    return filetext;
}

void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1, 1, 1, 0);

    glewInit();

    // VAO/VBO pentru axele de coordonate
    glGenBuffers(1, &vboAxes);
    glBindBuffer(GL_ARRAY_BUFFER, vboAxes);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), axes, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoAxes);
    glBindVertexArray(vaoAxes);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // VAO/VBO/EBO pentru sfera
    glGenBuffers(1, &vboSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER,
                 sphere.vertices.size() * sizeof(glm::vec3),
                 sphere.vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &eboSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sphereElementCount,
                 sphere.triangles.data(), GL_STATIC_DRAW);

    // Compilare shadere
    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");
    const char* vertex_shader   = vstext.c_str();
    const char* fragment_shader = fstext.c_str();

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

    printf("Press 1: sfera jumatate rosie / jumatate albastra (split dupa X=0)\n");
    printf("Press 2: culoare in functie de unghiul normalei cu axa OX\n");
}

float axisRotAngle = PI / 16.0f;
float radius = 2;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    GLuint matrixID = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
    // Setam uniformul `mode` pentru fragment shader sa citeasca in functie de valoarea stocata global
    GLuint modeID   = glGetUniformLocation(shader_programme, "mode");

    glUniform1i(modeID, currentExercise);

    // Initial desenam axele in scena direct in originile setate in `axes`
    glBindVertexArray(vaoAxes);
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
                       glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glDrawArrays(GL_LINES, 0, 6);

    // Selectam VBO-ul ce contine varfurile sferei unitare a noastre
    glBindVertexArray(vaoSphere);

    // Resetam la identitate pt fiecare afisare, nefiind obligatoriu sa preia vreo translatie pe care noi n-am setat-o
    modelMatrix = glm::mat4(1.0f);
    // Aplicam valoarea stocata prin eventul din keyboard ce se schimba interactiv "axisRotAngle" ca si rotatie pe axa Oy 
    modelMatrix *= glm::rotate(axisRotAngle, glm::vec3(0, 1, 0));
    // Daca aveam scale 1 si sfera era unitara trebuia in mod original sa aiba raza 1, ii dublam dimendiunile
    modelMatrix *= glm::scale(glm::vec3(2 * radius));

    // Pasam mai departe in shader matricea MVP totala aferenta transformari noastre din camera+dimensiuni obiect
    glUniformMatrix4fv(matrixID, 1, GL_FALSE,
                       glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    
    // Sfera unitara generata de `n` subdiviziuni de clasa auxiliara SphereMesh trebuie desenata dupa element array
    glDrawElements(GL_TRIANGLES, sphereElementCount, GL_UNSIGNED_INT, NULL);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 4, (float)w / h, 0.1f, 100.0f);
    viewMatrix = glm::lookAt(glm::vec3(xv, yv, zv),
                             glm::vec3(0, 0, 0),
                             glm::vec3(0, 1, 0));
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'a':
        axisRotAngle += 0.1f;
        if (axisRotAngle > 2 * PI) axisRotAngle = 0;
        break;
    case 's':
        axisRotAngle -= 0.1f;
        if (axisRotAngle < 0) axisRotAngle = 2 * PI;
        break;
    case '1':
        currentExercise = 1;
        printf("Exercise 1: half-half coloring\n");
        break;
    case '2':
        currentExercise = 2;
        printf("Exercise 2: normal-angle coloring\n");
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(700, 700);
    glutCreateWindow("SPG Lab7 - Sfera (1/2 pt exercitii)");
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
