#version 400
layout(location = 0) in vec3 vp;
uniform mat4 modelViewProjectionMatrix;
out vec3 position;
void main() {
	gl_Position = modelViewProjectionMatrix * vec4(vp, 1.0);
	position = vp;
}
