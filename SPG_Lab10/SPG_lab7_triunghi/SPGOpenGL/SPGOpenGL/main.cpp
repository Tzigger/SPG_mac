#include <fstream>
#include <string>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define STB_IMAGE_IMPLEMENTATION
#include <GL/stb_image.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>


int  currentExercise = 1;


bool useRepeat = true;

GLuint shader_programme;

// -----------------------------------------------------------------
// [ADAUGAT - Ex1, Ex4]
// Doua obiecte textura separate (texture objects):
//   texture1 = wall.jpg       (folosit in Ex1, Ex2, Ex3, Ex4)
//   texture2 = checkerboard   (creat procedural, folosit in Ex4)
// In OpenGL fiecare textura are un nume unic (GLuint) generat cu
// glGenTextures() si activat cu glBindTexture().
// -----------------------------------------------------------------
GLuint texture1;
GLuint texture2;

// =================================================================
// GEOMETRIE
// Fiecare vertex are: pozitie(3) + culoare RGB(3) + texCoord(2)
//                     = 8 floaturi per vertex  => stride = 8*sizeof(float)
// =================================================================

// -----------------------------------------------------------------
// [ADAUGAT - Ex1, Ex2]
// Triunghiul din cerinta laboratorului (Figura 1 din PDF):
//   vertex sus        → texCoord (0.5, 1.0)
//   vertex dreapta-jos→ texCoord (1.0, 0.0)
//   vertex stanga-jos → texCoord (0.0, 0.0)
// Culorile RGB per-vertex (rosu/verde/albastru) sunt vizibile in Ex2
// unde se combina cu textura prin inmultire.
// -----------------------------------------------------------------
GLuint vaoTri, vboTri;
float triVertices[] = {
    // pos              // color (RGB)       // texcoord [0,1]
     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.5f, 1.0f,   // sus    – rosu
     0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,   // dr-jos – verde
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f    // st-jos – albastru
};


GLuint vaoSq3, vboSq3;
float squareEx3[] = {
    // pos              // color (alb)       // texcoord [0,2] ← depasesc [0,1]
    -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 2.0f,
    -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  2.0f, 2.0f,

     0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  2.0f, 2.0f,
    -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  2.0f, 0.0f
};


GLuint vaoSq4, vboSq4;
float squareEx4[] = {
    // pos              // color (alb)       // texcoord [0,1]
    -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

     0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f
};

// =================================================================
std::string textFileRead(const char* fn)
{
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        filetext.append(line + "\n");
    }
    return filetext;
}

// -----------------------------------------------------------------
// [ADAUGAT - toate exercitiile]
// Functie helper care initializeaza un VAO + VBO cu stride fix de 8 floaturi:
//   - location 0: pozitie (3 floaturi, offset 0)
//   - location 1: culoare RGB (3 floaturi, offset 3*sizeof(float))
//   - location 2: coordonate textura UV (2 floaturi, offset 6*sizeof(float))
// Corespunde structurii din Figura 3 din PDF (interleaved vertex data).
// -----------------------------------------------------------------
void setupVAO(GLuint& vao, GLuint& vbo, float* data, size_t dataSize)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)dataSize, data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Atribut 0: pozitie xyz (3 floaturi), stride 8, offset 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float), (void*)0);

    // Atribut 1: culoare RGB (3 floaturi), stride 8, offset 3
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float), (void*)(3 * sizeof(float)));

    // Atribut 2: coordonate textura UV (2 floaturi), stride 8, offset 6
    // Aceasta este linia noua fata de Lab7: transmitem texCoord catre vertex shader.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float), (void*)(6 * sizeof(float)));
}

// =================================================================
void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glClearColor(1, 1, 1, 0);
    glewInit();

    setupVAO(vaoTri, vboTri, triVertices, sizeof(triVertices)); // Ex1, Ex2
    setupVAO(vaoSq3, vboSq3, squareEx3,  sizeof(squareEx3));   // Ex3
    setupVAO(vaoSq4, vboSq4, squareEx4,  sizeof(squareEx4));   // Ex4

    // =============================================================
    // [ADAUGAT - Ex1, Ex2, Ex3, Ex4]
    // TEXTURA 1: incarcam wall.jpg cu stbi_load()
    // Pasii obligatorii (conform PDF-ului):
    //   1. glGenTextures()   – generam un nume unic pentru textura
    //   2. glBindTexture()   – cream obiectul textura si il activam
    //   3. glTexParameteri() – setam wrap mode si filtrare
    //   4. stbi_load()       – citim pixelii imaginii de pe disc
    //   5. glTexImage2D()    – incarcam pixelii in GPU
    //   6. glGenerateMipmap()– generam automat lantul mipmap
    //   7. stbi_image_free() – eliberam memoria CPU
    // =============================================================
    
    
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("wall.jpg", &w, &h, &ch, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        printf("Texture 1 loaded: wall.jpg (%dx%d)\n", w, h);
    } else {
        printf("Failed to load texture: wall.jpg\n");
    }
    stbi_image_free(data);

    
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    {
        int w2, h2, ch2;
        unsigned char* data2 = stbi_load("texture-1874580_640.jpg", &w2, &h2, &ch2, 0);
        if (data2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w2, h2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
            glGenerateMipmap(GL_TEXTURE_2D);
            printf("Texture 2 loaded: texture-1874580_640.jpg (%dx%d)\n", w2, h2);
        } else {
            printf("Failed to load texture: texture-1874580_640.jpg\n");
        }
        stbi_image_free(data2);
    }

    // Incarcare si compilare shadere (neschimbata)
    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");
    const char* vs_src = vstext.c_str();
    const char* fs_src = fstext.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    { GLint ok; glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
      if (!ok) { char b[512]; glGetShaderInfoLog(vs, 512, NULL, b); printf("VS error:\n%s\n", b); } }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    { GLint ok; glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
      if (!ok) { char b[512]; glGetShaderInfoLog(fs, 512, NULL, b); printf("FS error:\n%s\n", b); } }

    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);
    { GLint ok; glGetProgramiv(shader_programme, GL_LINK_STATUS, &ok);
      if (!ok) { char b[512]; glGetProgramInfoLog(shader_programme, 512, NULL, b); printf("Link error:\n%s\n", b); } }

    printf("\nControale:\n");
    printf("  1 - Ex1: Triunghi texturat (wall.jpg)\n");
    printf("  2 - Ex2: Triunghi textura * culoare vertex\n");
    printf("  3 - Ex3: Patrat, texcoord [0,2], 'r'=REPEAT / 'c'=CLAMP_TO_EDGE\n");
    printf("  4 - Ex4: Patrat, doua texturi mixate cu mix()\n");
}

void applyWrapMode(GLint wrapMode)
{
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);

    glm::mat4 model(1.0f);
    GLuint matrixID  = glGetUniformLocation(shader_programme, "modelMatrix");
    GLuint modeID    = glGetUniformLocation(shader_programme, "mode");
    GLuint tex1ID    = glGetUniformLocation(shader_programme, "ourTexture");
    GLuint tex2ID    = glGetUniformLocation(shader_programme, "ourTexture2");

    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(model));

    if (currentExercise == 1) {
        glUniform1i(modeID, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glUniform1i(tex1ID, 0);
        glBindVertexArray(vaoTri);
        glDrawArrays(GL_TRIANGLES, 0, 3);

    } else if (currentExercise == 2) {
        glUniform1i(modeID, 2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glUniform1i(tex1ID, 0);
        glBindVertexArray(vaoTri);
        glDrawArrays(GL_TRIANGLES, 0, 3);

    } else if (currentExercise == 3) {
        applyWrapMode(useRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glUniform1i(modeID, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(tex1ID, 0);

        glBindVertexArray(vaoSq3);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    } else if (currentExercise == 4) {
        glUniform1i(modeID, 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(tex1ID, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(tex2ID, 1);

        glBindVertexArray(vaoSq4);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glFlush();
}

// =================================================================
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case '1': currentExercise = 1;
                  printf("Ex1 – Triunghi texturat\n"); break;
        case '2': currentExercise = 2;
                  printf("Ex2 – Triunghi: textura * culoare vertex\n"); break;
        case '3': currentExercise = 3;
                  printf("Ex3 – Patrat texcoord [0,2] | %s\n",
                         useRepeat ? "GL_REPEAT" : "GL_CLAMP_TO_EDGE"); break;
        case '4': currentExercise = 4;
                  printf("Ex4 – Patrat: mix(texture1, texture2, 0.5)\n"); break;
        case 'r': useRepeat = true;
                  if (currentExercise == 3) printf("Ex3 → GL_REPEAT\n"); break;
        case 'c': useRepeat = false;
                  if (currentExercise == 3) printf("Ex3 → GL_CLAMP_TO_EDGE\n"); break;
    }
    glutPostRedisplay();
}

// =================================================================
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(512, 512);
    glutCreateWindow("SPG Lab10 - Texturare (1/2/3/4)");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
