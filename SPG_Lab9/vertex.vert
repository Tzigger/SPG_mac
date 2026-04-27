#version 410 core

layout(location = 0) in vec3 vPos;    // pozitia vertexului (spatiu local)
layout(location = 1) in vec3 vNormal; // normala vertexului (spatiu local)

uniform mat4 modelViewProjectionMatrix; // MVP
uniform mat4 modelMatrix;               // M - pentru pozitia in world space
uniform mat4 normalMatrix;              // Transpose(Inverse(M)) - pentru normale

out vec3 fragPos; // pozitia in world space (pentru iluminare in fragment shader)
out vec3 normal;  // normala transformata in world space

void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(vPos, 1.0);

    // Pozitia in world space (w=1 pentru puncte)
    fragPos = vec3(modelMatrix * vec4(vPos, 1.0));

    // Normala transformata (w=0 pentru directii, normalMatrix corecteaza la scalari)
    normal = normalize(vec3(normalMatrix * vec4(vNormal, 0.0)));
}