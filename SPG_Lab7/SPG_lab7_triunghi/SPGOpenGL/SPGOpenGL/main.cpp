#include <fstream>
#include <string>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

int currentExercise = 1;

GLuint shader_programme;

// Folosim un singur VBO "interleaved" - stochează și coordonatele (x,y,z) și culoarea (r,g,b,a) consecutiv
GLuint vao1, vbo1;

float triangleData[] = {
    // coordonate (x, y, z)    // culoare (R, G, B, A)
     0.0f,  0.5f,  0.0f,       1.0f, 0.0f, 0.0f, 1.0f,  // Vertex sus – Roșu
     0.5f, -0.5f,  0.0f,       0.0f, 1.0f, 0.0f, 1.0f,  // Vertex dreapta-jos – Verde
    -0.5f, -0.5f,  0.0f,       0.0f, 0.0f, 1.0f, 1.0f   // Vertex stânga-jos – Albastru
};

// Exercițiul 2: Pătrat alcătuit din 2 triunghiuri
// 6 vertexuri totale ce conțin coordonate + culori unice (interleaved)
GLuint vao2, vbo2;

float squareData[] = {
    // Primul triunghi (sus-stânga, jos-stânga, sus-dreapta)
    -0.5f,  0.5f,  0.0f,   1.0f, 0.0f, 0.0f, 1.0f,  // stanga sus  – rosu
    -0.5f, -0.5f,  0.0f,   0.0f, 0.0f, 1.0f, 1.0f,  // stanga jos  – albastru
     0.5f,  0.5f,  0.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // dreapta sus – verde

    // Al doilea triunghi (sus-dreapta, jos-stânga, jos-dreapta)
     0.5f,  0.5f,  0.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // dreapta sus   – verde
    -0.5f, -0.5f,  0.0f,   0.0f, 0.0f, 1.0f, 1.0f,  // stanga jos    – albastru
     0.5f, -0.5f,  0.0f,   1.0f, 1.0f, 0.0f, 1.0f   // dreapta jos   – galben
};

// Exercițiul 3: Jumătate de triunghi roșu, jumătate albastru
// Alegem culoarea in fragment shader strict in functie de coordonata y.
GLuint vao3, vbo3;

float triHalfData[] = {
    // coordonate (x, y, z)    // culoare dummy, nefolosită
     0.0f,  0.5f,  0.0f,       0.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.0f,       0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f,       0.0f, 0.0f, 0.0f, 1.0f
};

std::string textFileRead(const char* fn)
{
    std::ifstream ifile(fn);
    std::string filetext;
    while (ifile.good()) {
        std::string line;
        std::getline(ifile, line);
        // Eliminam \r pentru compatibilitate cu fisiere Windows (CRLF)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        filetext.append(line + "\n");
    }
    return filetext;
}

// configuram cate 2 VertexAttribPointer
// pentru acelasi VBO: index 0 pentru pozitie (x,y,z), index 1 pentru culoare (r,g,b,a)
void setupVAO(GLuint& vao, GLuint& vbo, float* data, size_t dataSize)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)dataSize, data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Index 0 e pt pozitie (3 floaturi) - incepem de pe prima pozitie (offset 0)
    // Sariam peste 7 float-uri intre doua seturi (3 coordonate + 4 culori = stride)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          7 * sizeof(float), (void*)0);

    // Index 1 e pt culoare (4 floaturi) - incepem de la al 4-lea float (offset de 3 * sizeof(float))
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                          7 * sizeof(float), (void*)(3 * sizeof(float)));
}


void init()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glClearColor(1, 1, 1, 0);
    glewInit();

    // Configuram VAO/VBO 
    setupVAO(vao1, vbo1, triangleData, sizeof(triangleData)); // Ex 1
    setupVAO(vao2, vbo2, squareData,   sizeof(squareData));   // Ex 2
    setupVAO(vao3, vbo3, triHalfData,  sizeof(triHalfData));  // Ex 3

    // Incarcare shadere
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
    glAttachShader(shader_programme, vs);
    glAttachShader(shader_programme, fs);
    glLinkProgram(shader_programme);

    printf("Press 1: Exercise 1 - Colored triangle (RGB per vertex)\n");
    printf("Press 2: Exercise 2 - Square (2 triangles, RGBA per vertex)\n");
    printf("Press 3: Exercise 3 - Triangle half red / half blue\n");
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_programme);

    // Matricea de modelare: identitate 
    glm::mat4 model(1.0f);
    GLuint matrixID = glGetUniformLocation(shader_programme, "modelMatrix");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(model));

    // Incarcam in uniform valoarea int a modului selectat
    GLuint modeID = glGetUniformLocation(shader_programme, "mode");

    if (currentExercise == 1) {
        // Ex 1: Desenam primul VBO, continand cele 3 varfuri colorate manual
        glUniform1i(modeID, 1);
        glBindVertexArray(vao1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

    } else if (currentExercise == 2) {
        // Ex 2: Desenam cel de-al doilea VBO cu cele 6 varfuri
        glUniform1i(modeID, 2);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    } else if (currentExercise == 3) {
        // Ex 3: Desenam al 3-lea VBO (triunghi). Cand mode == 3 fragment shader-ul isi 
        // alege culoarea strict din y > 0
        glUniform1i(modeID, 3);
        glBindVertexArray(vao3);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == '1') { currentExercise = 1; printf("Exercise 1\n"); }
    if (key == '2') { currentExercise = 2; printf("Exercise 2\n"); }
    if (key == '3') { currentExercise = 3; printf("Exercise 3\n"); }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(512, 512);
    glutCreateWindow("SPG Lab7 - Triunghi (1/2/3 pt exercitii)");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
