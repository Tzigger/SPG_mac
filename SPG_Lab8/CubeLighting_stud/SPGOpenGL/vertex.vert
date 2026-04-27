#version 400

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 inNormal; // normala perpendiculara pe fata (din VBO)

uniform mat4 mvpMatrix;
uniform mat4 normalMatrix;
uniform int  mode; // 1 = normale pe diagonale, 2 = normale perpendiculare pe fete

out vec3 normal;
out vec3 pos;

void main()
{
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    vec3 rawNormal;
    if (mode == 1)
        // Normala pe directia diagonalei principale: de la centrul cubului spre vertex
        rawNormal = normalize(vPos);
    else
        // Normala perpendiculara pe fata, furnizata ca atribut
        rawNormal = inNormal;

    // Corectia normalelor: se aplica transpusa inversei matricii de modelare
    normal = normalize(vec3(normalMatrix * vec4(rawNormal, 0.0)));

    pos = vPos;
}