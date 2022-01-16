#version 430 core

uniform mat4 mvp;
uniform int render_type;

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 iv2tex_coord;
layout (location = 4) in vec3 iv3instance_vertex;

out VertexData {
	vec2 texcoord;
} vertexData;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main(void) {
	if(render_type == 0) {
		/*
		mat4 rm = rotationMatrix(vec3(0.0f, 0.0f, 1.0f), 180.0f);
		vec3 p = vec3(rm * vec4(position, 1.0f));

		gl_Position = mvp * vec4(p, 1.0);
		*/
		gl_Position = mvp * vec4(position, 1.0f);
	}
	else {
		mat4 rm = rotationMatrix(vec3(1.0f, 0.0f, 0.0f), -90.0f);
		vec3 p = vec3(rm * vec4(position, 1.0f));

		gl_Position = mvp * vec4(p + iv3instance_vertex, 1.0);
		vertexData.texcoord = iv2tex_coord;
	}
}