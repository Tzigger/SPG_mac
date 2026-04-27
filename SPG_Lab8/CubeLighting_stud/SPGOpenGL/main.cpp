#include <fstream>
#include <string>
#include <stdio.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#define PI glm::pi<float>()

GLuint shader_programme, vao;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

int nrFaces = 6;
int nrVerticesPerFace = 6;
int nrVertices = nrFaces * nrVerticesPerFace;

glm::vec3 lightPos(0, 1, 5);
glm::vec3 viewPos(2, 3, 6);

// Modul curent: 1 = normale pe diagonalele principale, 2 = normale perpendiculare pe fete
int currentMode = 1;

float L = 0.5f;

// Date entrelasate: pozitie (x,y,z) + normala perpendiculara pe fata (nx,ny,nz)
// Normala diagonala (normalize(vPos)) se calculeaza in vertex shader pentru mode=1
float cube[] = {
    // fata x pozitiva: normala = (1,0,0)
     L, -L, -L,   1, 0, 0,
     L, -L,  L,   1, 0, 0,
     L,  L,  L,   1, 0, 0,
     L, -L, -L,   1, 0, 0,
     L,  L, -L,   1, 0, 0,
     L,  L,  L,   1, 0, 0,

    // fata x negativa: normala = (-1,0,0)
    -L, -L, -L,  -1, 0, 0,
    -L,  L, -L,  -1, 0, 0,
    -L,  L,  L,  -1, 0, 0,
    -L, -L, -L,  -1, 0, 0,
    -L, -L,  L,  -1, 0, 0,
    -L,  L,  L,  -1, 0, 0,

    // fata y pozitiva: normala = (0,1,0)
     L,  L, -L,   0, 1, 0,
     L,  L,  L,   0, 1, 0,
    -L,  L,  L,   0, 1, 0,
     L,  L, -L,   0, 1, 0,
    -L,  L, -L,   0, 1, 0,
    -L,  L,  L,   0, 1, 0,

    // fata y negativa: normala = (0,-1,0)
     L, -L, -L,   0,-1, 0,
     L, -L,  L,   0,-1, 0,
    -L, -L,  L,   0,-1, 0,
     L, -L, -L,   0,-1, 0,
    -L, -L, -L,   0,-1, 0,
    -L, -L,  L,   0,-1, 0,

    // fata z pozitiva: normala = (0,0,1)
     L, -L,  L,   0, 0, 1,
     L,  L,  L,   0, 0, 1,
    -L,  L,  L,   0, 0, 1,
     L, -L,  L,   0, 0, 1,
    -L, -L,  L,   0, 0, 1,
    -L,  L,  L,   0, 0, 1,

    // fata z negativa: normala = (0,0,-1)
     L, -L, -L,   0, 0,-1,
     L,  L, -L,   0, 0,-1,
    -L,  L, -L,   0, 0,-1,
     L, -L, -L,   0, 0,-1,
    -L, -L, -L,   0, 0,-1,
    -L,  L, -L,   0, 0,-1,
};

std::string textFileRead(const char *fn)
{
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        // Strip Windows-style \r if present
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        filetext.append(line + "\n");
    }
    return filetext;
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

float rotAngle    = 0;
float rotAngleInc = PI / 64;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    modelMatrix = glm::mat4(1.0f);
    glBindVertexArray(vao);

    GLuint lightPosLoc = glGetUniformLocation(shader_programme, "lightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    GLuint viewPosLoc = glGetUniformLocation(shader_programme, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    modelMatrix *= glm::rotate(rotAngle, glm::vec3(0, 1, 0));

    GLuint modelMatrixLoc = glGetUniformLocation(shader_programme, "mvpMatrix");
    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE,
                       glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));

    // Matricea de corectie a normalelor: transpusa inversei matricii de modelare
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
    GLuint normalMatrixLoc = glGetUniformLocation(shader_programme, "normalMatrix");
    glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // Trimite modul curent catre vertex shader
    GLuint modeLoc = glGetUniformLocation(shader_programme, "mode");
    glUniform1i(modeLoc, currentMode);

    glDrawArrays(GL_TRIANGLES, 0, nrVertices);
    glFlush();
}

void init()
{
    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glClearColor(1, 1, 1, 0);
    glEnable(GL_DEPTH_TEST);
    glewInit();

    GLuint vbo = 1;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 6 floats per vertex: 3 pozitie + 3 normala
    glBufferData(GL_ARRAY_BUFFER, nrVertices * 6 * sizeof(float), cube, GL_STATIC_DRAW);

    vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Atribut 0: pozitia (x,y,z) - stride=6 floats, offset=0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);

    // Atribut 1: normala (nx,ny,nz) - stride=6 floats, offset=3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)(3 * sizeof(float)));

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

    printShaderInfoLog(vs);
    printShaderInfoLog(fs);
    printProgramInfoLog(shader_programme);

    printf("Taste:\n");
    printf("  a/s - rotatie stanga/dreapta\n");
    printf("  1   - normale pe diagonalele principale\n");
    printf("  2   - normale perpendiculare pe fete\n");
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 6, (float)w / h, 0.1f, 100.0f);
    viewMatrix = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'a':
        rotAngle += rotAngleInc;
        break;
    case 's':
        rotAngle -= rotAngleInc;
        break;
    case '1':
        currentMode = 1;
        printf("Mode 1: normale pe diagonalele principale\n");
        break;
    case '2':
        currentMode = 2;
        printf("Mode 2: normale perpendiculare pe fete\n");
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(700, 700);
    glutCreateWindow("SPG Lab8 - Iluminare Cub (Phong)");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
