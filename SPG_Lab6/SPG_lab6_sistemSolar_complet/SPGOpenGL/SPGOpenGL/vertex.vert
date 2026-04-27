#version 400
in vec3 vp;
uniform mat4 modelViewProjectionMatrix;

void main() 
{
	gl_Position = modelViewProjectionMatrix * vec4(vp, 1.0);
}
