#version 430 core

uniform mat4 mvp;
uniform int render_type;

layout (location = 0) in vec3 position;
layout (location = 4) in vec3 iv3instance_vertex;

void main(void) {
	if(render_type == 0)
		gl_Position = mvp * vec4(position, 1.0);
	else
		gl_Position = mvp * vec4(position + iv3instance_vertex, 1.0);
}