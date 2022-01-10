#version 410 core

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;


uniform mat4 shadow_matrix;
uniform mat4 um4mv;
uniform mat4 um4p;

uniform vec3 light_pos = vec3(0.2, 0.6, 0.5);
// uniform vec3 light_pos = vec3(1.0, 1.0, 1.0);

out VertexData
{
    vec3 N;
    vec3 L;
    vec3 V; 
    vec2 texcoord;
	vec3 iv3normal;
	vec4 shadow_coord;
} vertexData;

void main() {
	// phong shading
	vec4 P = um4mv * vec4(iv3vertex, 1.0);

	vertexData.N = mat3(um4mv) * iv3normal;
	vertexData.L = light_pos - P.xyz;
	vertexData.V = -P.xyz;

	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
	vertexData.iv3normal = iv3normal;
	vertexData.texcoord = iv2tex_coord;

	vertexData.shadow_coord = shadow_matrix * vec4(iv3vertex, 1.0);
}