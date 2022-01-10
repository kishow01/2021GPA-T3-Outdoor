#version 430 core

out vec4 fragColor;

void main() {
	fragColor = vec4(vec3(gl_FragCoord.y), 1.0);
}