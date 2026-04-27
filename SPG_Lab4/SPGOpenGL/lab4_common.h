#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei vertexCount = 0;
    GLsizei indexCount = 0;
    bool indexed = false;
};

extern std::string g_asset_dir;
extern const float kPi;

void setAssetDirFromArgv0(const char* argv0);
std::string textFileRead(const char* fn);

bool checkShader(GLuint shader, const char* label);
bool checkProgram(GLuint program);
GLuint buildShaderProgram(const char* vertexFile, const char* fragmentFile);

glm::mat4 customTranslation(float tx, float ty, float tz);
glm::mat4 customRotationZ(float radians);
glm::mat4 customScale(float sx, float sy, float sz);

void setupMesh(Mesh& mesh, const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
void generateSphere(float radius, int stacks, int slices, std::vector<float>& vertices, std::vector<unsigned int>& indices);
void drawMesh(const Mesh& mesh);
