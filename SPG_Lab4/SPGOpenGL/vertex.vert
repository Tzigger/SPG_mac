#version 410
layout(location = 0) in vec3 vp;
uniform mat4 modelMatrix;


uniform vec2 uOffset;
uniform float uPulse;
uniform int useShaderTransform;

void main() {
	vec4 p = modelMatrix * vec4(vp, 1.0);

	if (useShaderTransform == 1) {
		p.xy += uOffset;
		p.xyz *= (1.0 + 0.15 * uPulse);
	}
	gl_Position = p;
}