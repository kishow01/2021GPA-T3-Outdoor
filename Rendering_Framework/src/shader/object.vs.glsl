#version 410 core

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;
layout(location = 2) in vec3 iv3tangent;
layout(location = 3) in vec2 iv2tex_coord;

uniform int render_type;
uniform bool normal_mapping_enable;

uniform mat4 shadow_matrix;
uniform mat4 um4mv;
uniform mat4 um4p;

uniform vec3 light_pos = vec3(0.2, 0.6, 0.5);
// uniform vec3 light_pos = vec3(1.0, 1.0, 1.0);

out VertexData
{
	// phong_shading_rendering
    vec3 N;
    vec3 L;
    vec3 V; 
    vec2 texcoord;
	vec3 iv3normal;

	// normal_mapping_render
	vec3 lightDir;
	vec3 eyeDir;

	vec4 shadow_coord;
} vertexData;

void phong_shading_rendering() {
	vec4 P = um4mv * vec4(iv3vertex, 1.0);

	vertexData.N = mat3(um4mv) * iv3normal;
	vertexData.L = light_pos - P.xyz;
	vertexData.V = -P.xyz;

	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
	vertexData.iv3normal = iv3normal;
	vertexData.texcoord = iv2tex_coord;

	vertexData.shadow_coord = shadow_matrix * vec4(iv3vertex, 1.0);
}

void normal_mapping_render() {
	// Calculate vertex position in view space.
	vec4 P = um4mv * vec4(iv3vertex, 1.0);

	// Calculate normal (N) and tangent (T) vectors in view space from
	// incoming object space vectors.
	vec3 V = P.xyz; 
	vec3 N = normalize(mat3(um4mv) * iv3normal); 
	vec3 T = normalize(mat3(um4mv) * iv3tangent);

	// Calculate the bitangent vector (B) from the normal and tangent 
	// vectors.
	vec3 B = cross(N, T);

	// The light vector (L) is the vector from the point of interest to
    // the light. Calculate that and multiply it by the TBN matrix
    vec3 L = light_pos - P.xyz;
    vertexData.lightDir = normalize(vec3(dot(L, T), dot(L, B), dot(L, N)));

	// The view vector is the vector from the point of interest to the
    // viewer, which in view space is simply the negative of the position.
    // Calculate that and multiply it by the TBN matrix.
    V = -P.xyz;
    vertexData.eyeDir = normalize(vec3(dot(V, T), dot(V, B), dot(V, N)));  

	vertexData.texcoord = iv2tex_coord;
	vertexData.iv3normal = iv3normal;

	gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
	vertexData.shadow_coord = shadow_matrix * vec4(iv3vertex, 1.0);
}

void main() {
	if(render_type == 0)
		phong_shading_rendering();
	else if(render_type == 1) {
		if(normal_mapping_enable)
			normal_mapping_render();
		else
			phong_shading_rendering();
	} 
}