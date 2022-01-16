#version 410 core

uniform sampler2D tex;
uniform sampler2D brightFilter_tex;

layout (location = 0) out vec4 color;

uniform int bloom_option;

in VS_OUT {
    vec2 texcoord;
} fs_in;

void main() {
	// color = vec4(texture(tex, fs_in.texcoord).rgb, 1.0);
	// color = vec4(texture(brightFilter_tex, fs_in.texcoord).rgb, 1.0);
	
	// Gaussian Blur on brightFilter color
	int half_size = 2;
    vec4 color_sum = vec4(0);
	for(int i = -half_size; i <= half_size; i++) {
        for(int j = -half_size; j <= half_size; j++) {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(brightFilter_tex, coord, 0);
        }
    }

	int sampler_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    vec4 gaussian_color = color_sum / sampler_count;

	
	if(bloom_option == 0)
		color = vec4(gaussian_color.rgb + texture(tex, fs_in.texcoord).rgb, 1.0);
	else if(bloom_option == 1)
		color = vec4(texture(tex, fs_in.texcoord).rgb, 1.0);
	else if(bloom_option == 2)
		color = vec4(gaussian_color.rgb, 1.0);
}