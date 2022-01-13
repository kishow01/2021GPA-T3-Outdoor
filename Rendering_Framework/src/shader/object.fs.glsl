#version 420

uniform sampler2DShadow shadow_tex;
uniform sampler2D tex;
uniform sampler2D normal_tex;

out vec4 color;

uniform int render_type;
uniform bool normal_mapping_enable;

uniform mat4 um4mv;
uniform mat4 um4p;

vec3 Ia = vec3(0.1);
vec3 Id = vec3(0.8);
vec3 Is = vec3(0.1);
float specular_power = 100.0;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

in VertexData
{
    vec3 N;
    vec3 L;
    vec3 V; 
    vec2 texcoord;
	vec3 iv3normal;

	vec3 lightDir;
	vec3 eyeDir;

	vec4 shadow_coord;
} vertexData;

void phong_shading_rendering() {
	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);

	vec3 ambient = texture(tex, vertexData.texcoord).rgb * Ia;
	vec3 diffuse = texture(tex, vertexData.texcoord).rgb * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), specular_power);

	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	color = vec4(ambient, 1.0) + shadow_factor * vec4(diffuse + specular, 1.0);

	// color = vec4(ambient + diffuse + specular, 1.0);
}

void normal_mapping_render() {
	// Normalize our incomming view and light direction vectors.
	vec3 V = normalize(vertexData.eyeDir);
    vec3 L = normalize(vertexData.lightDir);

    // Read the normal from the normal map and normalize it.
    vec3 N = normalize(texture(normal_tex, vertexData.texcoord).rgb * 2.0 - vec3(1.0));

    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-L, N);

	vec3 ambient = texture(tex, vertexData.texcoord).rgb * Ia;
	vec3 diffuse = texture(tex, vertexData.texcoord).rgb * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(R, V), 0.0), specular_power);

	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	color = vec4(ambient, 1.0) + shadow_factor * vec4(diffuse + specular, 1.0);

	// color = vec4(ambient + diffuse + specular, 1.0);
}

void tree_rendering() {
	float alpha = texture(tex, vertexData.texcoord).a;
	if(alpha < 0.1)
		discard;

	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);

	vec3 ambient = texture(tex, vertexData.texcoord).rgb * Ia;
	vec3 diffuse = texture(tex, vertexData.texcoord).rgb * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), specular_power);

	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	color = vec4(ambient, 1.0) + shadow_factor * vec4(diffuse + specular, 1.0);

	// color = vec4(ambient + diffuse + specular, 1.0);
}

void grass_rendering() {
	float alpha = texture(tex, vertexData.texcoord).a;
	if(alpha < 0.1)
		discard;

	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);

	vec3 ambient = texture(tex, vertexData.texcoord).rgb * Ia;
	vec3 diffuse = texture(tex, vertexData.texcoord).rgb * Id * max(dot(N, L), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(N, H), 0.0), specular_power);

	color = vec4(ambient + diffuse + specular, 1.0);
}

void main() {
	if(render_type == 0)
		phong_shading_rendering();
	else if(render_type == 1) {
		if(normal_mapping_enable)
			normal_mapping_render();
		else
			phong_shading_rendering();
	} else if (render_type == 2 || render_type == 3 || render_type == 6 || render_type == 7) {
		tree_rendering();
	} else if (render_type == 4 || render_type == 5) {
		grass_rendering();
		// phong_shading_rendering();
	}
}