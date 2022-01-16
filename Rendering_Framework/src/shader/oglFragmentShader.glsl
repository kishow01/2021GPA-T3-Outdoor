#version 430 core

in vec3 f_viewVertex ;
in vec3 f_uv ;
in vec4 shadow_coord;

// phong shading & brightFilterColor
in vec3 N;
in vec3 L;
in vec3 V; 

layout (location = 0) out vec4 fragColor0 ;
layout (location = 1) out vec4 brightFilterColor;

uniform sampler2DShadow shadow_tex;
uniform sampler2D texture0 ;

vec4 withFog(vec4 color){
	const vec4 FOG_COLOR = vec4(1.0, 1.0, 1.0, 1) ;
	const float MAX_DIST = 1000.0 ;
	const float MIN_DIST = 600.0 ;
	
	float dis = length(f_viewVertex) ;
	float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST) ;
	fogFactor = clamp(fogFactor, 0.0, 1.0) ;
	fogFactor = fogFactor * fogFactor ;
	
	vec4 colorWithFog = mix(FOG_COLOR, color, fogFactor) ;
	return colorWithFog ;
}

void renderTerrain(){
	// get terrain color
	vec4 terrainColor = texture(texture0, f_uv.rg) ;				
	// apply fog
	vec4 fColor = withFog(terrainColor) ;
	fColor.a = 1.0 ;
	fragColor0 = fColor ;

	float shadow_factor = textureProj(shadow_tex, shadow_coord);
	
	fragColor0 = fragColor0 * shadow_factor;

	// phong shading & brightFilterColor
	vec3 Ia = vec3(0.1);
	vec3 Id = vec3(1.2);
	vec3 Is = vec3(0.1);
	float specular_power = 100.0;
	vec3 Ks = vec3(0.0);

	vec3 n = normalize(N);
	vec3 l = normalize(L);
	vec3 v = normalize(V);
	vec3 h = normalize(l + v);

	vec3 ambient = texture(texture0, f_uv.rg).rgb * Ia;
	vec3 diffuse = texture(texture0, f_uv.rg).rgb * Id * max(dot(n, l), 0.0);
	vec3 specular = Ks * Is * pow(max(dot(n, h), 0.0), specular_power);

	float d = length(L);
	float attenuation = clamp( 10.0 / d, 0.0, 1.0);

	vec3 ps_color = attenuation * (ambient + diffuse + specular);
	float brightness = (ps_color.r + ps_color.g + ps_color.b) / 3.0f;
	brightFilterColor = vec4(brightness * ps_color, 1.0);
}

void main(){	
	renderTerrain() ;
}