#version 410

// -------------------------------------------------------
// Atribut 0: pozitia vertexului pe sfera unitara
//   Pe o sfera unitara centrata in origine,
//   pozitia vertexului = normala la suprafata
// -------------------------------------------------------
layout(location = 0) in vec3 vp;

uniform mat4 modelViewProjectionMatrix;

// Pozitia (normala) pasata la fragment shader
// pentru a determina culoarea in exercitiile 1 si 2
out vec3 vPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(vp, 1.0);

    // Transmite pozitia vertexului (= normala sferei) la fragment shader
    // Exercitiul 1: se foloseste semnul componentei X
    // Exercitiul 2: se foloseste valoarea componentei X (= cos unghi cu OX)
    vPosition = vp;
}
