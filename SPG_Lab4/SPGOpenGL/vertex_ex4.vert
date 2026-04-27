#version 410
layout(location = 0) in vec3 vp;

uniform float scaleFactor;
uniform vec3 translationVec;
uniform float rotationAngle;

void main() {
    vec3 p = vp * scaleFactor;

    float c = cos(rotationAngle);
    float s = sin(rotationAngle);

    vec3 rotated;
    rotated.x = c * p.x - s * p.y;
    rotated.y = s * p.x + c * p.y;
    rotated.z = p.z;

    gl_Position = vec4(rotated + translationVec, 1.0);
}
