#version 420

uniform sampler2DShadow shadow_tex;
uniform sampler2D tex;

out vec4 color;

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
	vec4 shadow_coord;
} vertexData;

void main()
{
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