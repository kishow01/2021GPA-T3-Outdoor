#version 420

uniform sampler2DShadow shadow_tex;
uniform sampler2D tex;
uniform sampler2D normal_tex;

out vec4 color;

uniform int render_type;
uniform bool normal_mapping_enable;

uniform mat4 um4mv;
uniform mat4 um4p;

vec3 Ka = vec3(1.000000, 1.000000, 1.000000);
vec3 Kd = vec3(0.800000, 0.800000, 0.800000);
vec3 Ks = vec3(0.500000, 0.500000, 0.500000);

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

	vec3 ambient = texture(tex, vertexData.texcoord).rgb;
	vec3 diffuse_albedo = ambient;
	vec3 specular_albedo = vec3(1.0);
	float specular_power = 100.0;

	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

	color = vec4(ambient * 0.1 + diffuse * 0.8 + specular * 0.1, 1.0);

	// color = texture(tex, vertexData.texcoord).rgba;
	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	if(shadow_factor < 0.5) {
		// Render Shadow Color
		color = vec4(0.41, 0.36, 0.37, 1.0) + color;
	}
}

void normal_mapping_render() {
	// Normalize our incomming view and light direction vectors.
	vec3 V = normalize(vertexData.eyeDir);
    vec3 L = normalize(vertexData.lightDir);

    // Read the normal from the normal map and normalize it.
    vec3 N = normalize(texture(normal_tex, vertexData.texcoord).rgb * 2.0 - vec3(1.0));

    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-L, N);

	vec3 ambient = texture(tex, vertexData.texcoord).rgb;
	vec3 diffuse_albedo = ambient;
	vec3 specular_albedo = vec3(1.0);
	float specular_power = 100.0;


	// Calculate diffuse color with simple N dot L.
	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;

	// Calculate Phong specular highlight
	// vec3 specular = max(pow(dot(R, V), 20.0), 0.0) * specular_albedo;
	vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;

	color = vec4(ambient * 0.1 + diffuse * 0.8 + specular * 0.1, 1.0);
	
	float shadow_factor = textureProj(shadow_tex, vertexData.shadow_coord);
	if(shadow_factor < 0.5) {
		// Render Shadow Color
		color = vec4(0.41, 0.36, 0.37, 1.0) + color;
	}
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