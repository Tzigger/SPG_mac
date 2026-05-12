#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cstdio>
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

GLuint shader_programme;
GLuint squareVAO, squareVBO;
GLuint sphereVAO, sphereVBO, sphereEBO;
GLuint colorTex, normalMapTex, noiseTex;

int currentScene = 0; // 0 square, 1 sphere
bool useNormalMapping = false;
float squareMix = 1.0f;      // coeficient mix normala clasica <-> normala din normal map (patrat)
float noiseDelta = 0.005f;   // pas pentru aproximarea gradientului de noise (sfera)
float noiseStrength = 2.0f;  // amplitudinea perturbarii normalei pe sfera

glm::mat4 projection, view, model;

float square[] = {
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
};

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;

void buildSphere(unsigned int sectors, unsigned int stacks)
{
    sphereVertices.clear();
    sphereIndices.clear();

    for (unsigned int i = 0; i <= stacks; ++i) {
        // Parametrizare sferica: generam vertecsi pe "paralele" si "meridiane"
        float v = (float)i / (float)stacks;
        float stackAngle = glm::pi<float>() * (v - 0.5f);
        float xy = cosf(stackAngle);
        float z = sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float u = (float)j / (float)sectors;
            float sectorAngle = u * 2.0f * glm::pi<float>();
            float x = xy * sinf(sectorAngle);
            float y = z;
            float zz = xy * cosf(sectorAngle);

            sphereVertices.push_back(x * 0.55f);
            sphereVertices.push_back(y * 0.55f);
            sphereVertices.push_back(zz * 0.55f);
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(zz);
            sphereVertices.push_back(0.0f);
            sphereVertices.push_back(0.0f);
        }
    }

    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphereIndices.push_back(k1);
                sphereIndices.push_back(k2);
                sphereIndices.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                sphereIndices.push_back(k1 + 1);
                sphereIndices.push_back(k2);
                sphereIndices.push_back(k2 + 1);
            }
        }
    }
}

GLuint loadTextureRGB(const char* fileName)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, nrChannels;
    unsigned char* data = stbi_load(fileName, &w, &h, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture: " << fileName << std::endl;
        return tex;
    }

    // Suport pentru imagini cu 1, 3 sau 4 canale
    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_RED;
    if (nrChannels == 4) format = GL_RGBA;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // evita probleme de aliniere la upload (important pe macOS)
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

GLuint loadTextureRed(const char* fileName)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, nrChannels;
    unsigned char* data = stbi_load(fileName, &w, &h, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture: " << fileName << std::endl;
        return tex;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

void setupSquare()
{
    // Layout vertecsi: pozitie(3), normala(3), texCoord(2)
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);

    glBindVertexArray(squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void setupSphere()
{
    buildSphere(64, 64); // rezolutie geometrie sfera

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    model = glm::mat4(1.0f);

    GLint modelID = glGetUniformLocation(shader_programme, "model");
    GLint viewID = glGetUniformLocation(shader_programme, "view");
    GLint projID = glGetUniformLocation(shader_programme, "projection");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f(glGetUniformLocation(shader_programme, "lightPos"), 1.2f, 0.8f, 1.8f);
    glUniform3f(glGetUniformLocation(shader_programme, "viewPos"), 0.0f, 0.0f, 2.0f);

    // Uniforme de control 
    glUniform1i(glGetUniformLocation(shader_programme, "sceneType"), currentScene);
    glUniform1i(glGetUniformLocation(shader_programme, "useNormalMap"), useNormalMapping ? 1 : 0);
    glUniform1f(glGetUniformLocation(shader_programme, "squareMix"), squareMix);
    glUniform1f(glGetUniformLocation(shader_programme, "noiseDelta"), noiseDelta);
    glUniform1f(glGetUniformLocation(shader_programme, "noiseStrength"), noiseStrength);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glUniform1i(glGetUniformLocation(shader_programme, "colorTex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMapTex);
    glUniform1i(glGetUniformLocation(shader_programme, "normalMapTex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glUniform1i(glGetUniformLocation(shader_programme, "noiseTex"), 2);

    if (currentScene == 0) {
        // Ex.1: patrat (GL_TRIANGLE_FAN)
        glBindVertexArray(squareVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } else {
        // Ex.2: sfera (indexed draw)
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0);
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    float aspect = (h == 0) ? 1.0f : (float)w / (float)h;
    projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y)
{
    (void)x; (void)y;

    // Selectie rapida demo
    if (key == '1') { currentScene = 0; useNormalMapping = false; } // patrat fara NM
    if (key == '2') { currentScene = 0; useNormalMapping = true; }  // patrat cu NM
    if (key == '3') { currentScene = 1; useNormalMapping = false; } // sfera fara NM
    if (key == '4') { currentScene = 1; useNormalMapping = true; }  // sfera cu NM noise

    if (key == 'q') squareMix = glm::clamp(squareMix - 0.1f, 0.0f, 1.0f); // problema propusa: control mix
    if (key == 'e') squareMix = glm::clamp(squareMix + 0.1f, 0.0f, 1.0f);

    if (key == 'z') noiseDelta = glm::clamp(noiseDelta - 0.001f, 0.001f, 0.02f);
    if (key == 'x') noiseDelta = glm::clamp(noiseDelta + 0.001f, 0.001f, 0.02f);

    if (key == 'c') noiseStrength = glm::clamp(noiseStrength - 0.2f, 0.0f, 5.0f);
    if (key == 'v') noiseStrength = glm::clamp(noiseStrength + 0.2f, 0.0f, 5.0f);

    std::printf("Scene=%d useNM=%d squareMix=%.2f noiseDelta=%.3f noiseStrength=%.2f\n",
        currentScene, useNormalMapping ? 1 : 0, squareMix, noiseDelta, noiseStrength);

    glutPostRedisplay();
}

GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen + 1);
        glGetShaderInfoLog(shader, logLen, NULL, log.data());
        std::printf("Shader compile error: %s\n", log.data());
    }
    return shader;
}

void init()
{
    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");

    GLuint vs = compileShader(GL_VERTEX_SHADER, vstext.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fstext.c_str());

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.93f, 0.93f, 0.95f, 1.0f);

    stbi_set_flip_vertically_on_load(true);

    // Ex.1
    colorTex = loadTextureRGB("brickwall.jpg");
    normalMapTex = loadTextureRGB("brickwall_normal.jpg");
    
    // Ex.2 (noise grayscale -> canal RED)
    noiseTex = loadTextureRed("noise.jpg");

    setupSquare();
    setupSphere();

    std::printf("\nTaste:\n");
    std::printf("1 - Patrat fara normal mapping\n");
    std::printf("2 - Patrat cu normal mapping (map + mix)\n");
    std::printf("3 - Sfera fara normal mapping\n");
    std::printf("4 - Sfera cu noise normal mapping\n");
    std::printf("q/e - scade/creste mix pe patrat [0..1]\n");
    std::printf("z/x - scade/creste delta pentru gradient noise\n");
    std::printf("c/v - scade/creste intensitatea asperitatilor\n");
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200, 100);
    glutInitWindowSize(900, 700);
    glutCreateWindow("SPG Lab12 - Normal Mapping");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
