#version 430 core

in vec3 f_viewVertex ;
in vec3 f_uv ;
in vec4 shadow_coord;

layout (location = 0) out vec4 fragColor0 ;

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
	if(shadow_factor < 0.5) {
		// Render Shadow Color
		fragColor0 = fragColor0 * vec4(0.41, 0.36, 0.37, 1.0);
	}
}

void main(){	
	renderTerrain() ;
}