#version 430 core

out vec4 fragColor;

uniform sampler2D tex;
uniform int render_type;

in VertexData {
	vec2 texcoord;
} vertexData;

void main() {
	if(render_type == 0 || render_type == 1)
		fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	else {
		float alpha = texture(tex, vertexData.texcoord).a;
		if(alpha < 0.1)
			discard;

		fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	}
}