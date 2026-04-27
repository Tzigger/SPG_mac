#include "lab4_common.h"

#include <cmath>
#include <fstream>
#include <string>
#include <vector>

#include <stdio.h>

#include <glm/gtc/constants.hpp>

std::string g_asset_dir = ".";
const float kPi = glm::pi<float>();

void setAssetDirFromArgv0(const char* argv0)
{
    if (argv0 == NULL) {
        return;
    }

    std::string exePath(argv0);
    size_t lastSlash = exePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        g_asset_dir = exePath.substr(0, lastSlash);
    }
}

std::string textFileRead(const char* fn)
{
    std::ifstream ifile(fn);
    if (!ifile.is_open()) {
        std::string fallback = g_asset_dir + "/" + fn;
        ifile.open(fallback.c_str());
        if (!ifile.is_open()) {
            fprintf(stderr, "Could not open shader file: %s (also tried %s)\n", fn, fallback.c_str());
            return "";
        }
    }

    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        filetext.append(line + "\n");
    }
    return filetext;
}

bool checkShader(GLuint shader, const char* label)
{
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE) {
        return true;
    }

    GLint logLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0) {
        std::string log(logLen, '\0');
        glGetShaderInfoLog(shader, logLen, NULL, &log[0]);
        fprintf(stderr, "%s compile error:\n%s\n", label, log.c_str());
    }
    return false;
}

bool checkProgram(GLuint program)
{
    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) {
        return true;
    }

    GLint logLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0) {
        std::string log(logLen, '\0');
        glGetProgramInfoLog(program, logLen, NULL, &log[0]);
        fprintf(stderr, "Program link error:\n%s\n", log.c_str());
    }
    return false;
}

GLuint buildShaderProgram(const char* vertexFile, const char* fragmentFile)
{
    std::string vstext = textFileRead(vertexFile);
    std::string fstext = textFileRead(fragmentFile);

    if (vstext.empty() || fstext.empty()) {
        fprintf(stderr, "Shader sources are empty.\n");
        return 0;
    }

    const char* vertex_shader = vstext.c_str();
    const char* fragment_shader = fstext.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    if (!checkShader(vs, "Vertex shader")) {
        glDeleteShader(vs);
        return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    if (!checkShader(fs, "Fragment shader")) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, fs);
    glAttachShader(program, vs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!checkProgram(program)) {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

glm::mat4 customTranslation(float tx, float ty, float tz)
{
    glm::mat4 m(1.0f);
    m[3][0] = tx;
    m[3][1] = ty;
    m[3][2] = tz;
    return m;
}

glm::mat4 customRotationZ(float radians)
{
    float c = std::cos(radians);
    float s = std::sin(radians);

    glm::mat4 m(1.0f);
    m[0][0] = c;
    m[1][0] = -s;
    m[0][1] = s;
    m[1][1] = c;
    return m;
}

glm::mat4 customScale(float sx, float sy, float sz)
{
    glm::mat4 m(1.0f);
    m[0][0] = sx;
    m[1][1] = sy;
    m[2][2] = sz;
    return m;
}

void setupMesh(Mesh& mesh, const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (!indices.empty()) {
        glGenBuffers(1, &mesh.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        mesh.indexed = true;
        mesh.indexCount = (GLsizei)indices.size();
    } else {
        mesh.indexed = false;
        mesh.vertexCount = (GLsizei)(vertices.size() / 3);
    }

    glBindVertexArray(0);
}

void generateSphere(float radius, int stacks, int slices, std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / (float)stacks;
        float phi = -kPi * 0.5f + v * kPi;
        float y = std::sin(phi);
        float r = std::cos(phi);

        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / (float)slices;
            float theta = u * 2.0f * kPi;
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int first = (unsigned int)(i * (slices + 1) + j);
            unsigned int second = first + (unsigned int)slices + 1U;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1U);

            indices.push_back(second);
            indices.push_back(second + 1U);
            indices.push_back(first + 1U);
        }
    }
}

void drawMesh(const Mesh& mesh)
{
    glBindVertexArray(mesh.vao);
    if (mesh.indexed) {
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, NULL);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    }
}
