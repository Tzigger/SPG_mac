#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>

#include <stack>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI glm::pi<float>()

glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

std::string textFileRead(const char* fn) {
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        filetext.append(line + "\n");
    }
    return filetext;
}

float points[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f
};

GLuint shader_programme, vao;
GLuint vao2, vbo, vbo2;
int height, width;
glm::mat4 projection, view, model, mvp;

int currentExercise = 1;
bool reverseOrder = false;

GLuint texture1, texture2;

GLuint modeID, alphaID, textureIndexID;

float vertices[] = {
     // positions         // colors           // texture coords
     0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.5f, 1.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f
};

// doua triunghiuri suprapuse partial (6 varfuri)
float vertices2[] = {
    // triunghi spate (mov, opac)
    -0.90f, -0.20f, 0.0f,   0.62f, 0.20f, 0.85f,   0.0f, 0.0f,
     0.10f,  0.85f, 0.0f,   0.62f, 0.20f, 0.85f,   1.0f, 1.0f,
     0.80f, -0.75f, 0.0f,   0.62f, 0.20f, 0.85f,   1.0f, 0.0f,

    // triunghi fata (verde, semitransparent)
    -0.75f,  0.70f, 0.0f,   0.20f, 0.82f, 0.35f,   0.0f, 1.0f,
     0.92f,  0.15f, 0.0f,   0.20f, 0.82f, 0.35f,   1.0f, 1.0f,
    -0.10f, -0.88f, 0.0f,   0.20f, 0.82f, 0.35f,   0.2f, 0.0f
};

void setupVAO(GLuint& vaoLocal, GLuint& vboLocal, float* data, int countFloats)
{
    glGenBuffers(1, &vboLocal);
    glBindBuffer(GL_ARRAY_BUFFER, vboLocal);
    glBufferData(GL_ARRAY_BUFFER, countFloats * sizeof(float), data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoLocal);
    glBindVertexArray(vaoLocal);
    glBindBuffer(GL_ARRAY_BUFFER, vboLocal);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

GLuint loadTexture(const char* fileName)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int w, h, nrChannels;
    unsigned char* data = stbi_load(fileName, &w, &h, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << fileName << std::endl;
    }
    stbi_image_free(data);

    return tex;
}

void drawOverlapTriangles(int texMode)
{
    // texMode: 0 = doar culori, 1 = texturat
    if (!reverseOrder) {
        // spate apoi fata
        glUniform1i(textureIndexID, 0); // textura 1 pentru triunghiul din spate
        glUniform1f(alphaID, (texMode == 1) ? 0.7f : 1.0f);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniform1i(textureIndexID, 1); // textura 2 pentru triunghiul din fata
        glUniform1f(alphaID, 0.4f);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 3, 3);
    }
    else {
        // fata apoi spate
        glUniform1i(textureIndexID, 1); // textura 2 pentru triunghiul din fata
        glUniform1f(alphaID, 0.4f);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 3, 3);

        glUniform1i(textureIndexID, 0); // textura 1 pentru triunghiul din spate
        glUniform1f(alphaID, (texMode == 1) ? 0.7f : 1.0f);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    model = glm::rotate(glm::radians(10.0f), glm::vec3(0, 0, 1));
    mvp = projection * view * model;

    GLuint matrixID = glGetUniformLocation(shader_programme, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    if (currentExercise == 1) {
        glDisable(GL_BLEND);
        glUniform1i(modeID, 1); // mix
        glUniform1f(alphaID, 1.0f);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else if (currentExercise == 2) {
        glDisable(GL_BLEND);
        glUniform1i(modeID, 2); // jumatate-jumatate
        glUniform1f(alphaID, 1.0f);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    else if (currentExercise == 3) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(modeID, 3); // culori
        drawOverlapTriangles(0);
    }
    else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(modeID, 4); // texturi
        drawOverlapTriangles(1);
    }

    glutSwapBuffers();
}

void init()
{
    glClearColor(1, 1, 1, 0);
}

void reshape(int w, int h)
{
    projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);

    view = glm::lookAt(
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );

    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y)
{
    (void)x; (void)y;

    if (key == '1') currentExercise = 1;
    if (key == '2') currentExercise = 2;
    if (key == '3') currentExercise = 3;
    if (key == '4') currentExercise = 4;
    if (key == '0') reverseOrder = !reverseOrder;

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(512, 512);
    glutCreateWindow("SPG");

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    GLint texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    printf("Max number of textures =  %d\n", texture_units);

    glewInit();
    init();

    stbi_set_flip_vertically_on_load(true);
    texture1 = loadTexture("wallg.jpg");
    texture2 = loadTexture("texture-1874580_640.jpg");

    setupVAO(vao, vbo, vertices, 24);
    setupVAO(vao2, vbo2, vertices2, 48);

    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");
    const char* vertex_shader = vstext.c_str();
    const char* fragment_shader = fstext.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLint status = GL_FALSE;
    int InfoLogLength;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(vs, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    status = GL_FALSE;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(fs, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);

    glUseProgram(shader_programme);

    GLuint tex1ID = glGetUniformLocation(shader_programme, "texture1");
    GLuint tex2ID = glGetUniformLocation(shader_programme, "texture2");
    glUniform1i(tex1ID, 0);
    glUniform1i(tex2ID, 1);

    modeID = glGetUniformLocation(shader_programme, "mode");
    alphaID = glGetUniformLocation(shader_programme, "alphaValue");
    textureIndexID = glGetUniformLocation(shader_programme, "useTexturePair");

    printf("\nTaste:\n");
    printf("1 - Ex1 mix doua texturi\n");
    printf("2 - Ex1 jumatate/jumatate\n");
    printf("3 - Ex2 doua triunghiuri colorate cu blending\n");
    printf("4 - Ex3 doua triunghiuri texturate cu blending (2 texturi diferite)\n");
    printf("0 - schimba ordinea de desenare\n");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
