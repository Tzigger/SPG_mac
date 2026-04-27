#version 410

// Citim pozitia din `location = 0` unde avem coordonatele varfului
layout(location = 0) in vec3 vp;

// Citim culoarea manuala setata per-vertex (prezenta in Ex. 0 si 1) din `location = 1`
layout(location = 1) in vec4 color;

uniform mat4 modelMatrix;

// Variabila pentru a pasa la fragment shader culoarea citita din VBO (Ex. 0, 1)
out vec4 vColor;

// Variabila pentru a pasa pozitia la fragment shader (Ex. 2)
out vec3 vPosition;

void main() {
    gl_Position = modelMatrix * vec4(vp, 1.0);

    // Transferam mai departe culoarea catre fragment shader 
    vColor = color;

    // Folosim in plus vPosition in fragment shader pentru comparatia y > 0
    vPosition = vp;
}