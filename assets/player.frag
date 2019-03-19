#version 430 core

in vec2 Tex;

layout(binding = 0) uniform sampler2D CharFont;
layout(location = 1) uniform bool Hidden; // True if rendering with GL_DEPTH_FUNC set to GL_GREATER

const vec3 HIDDEN_COLOR = vec3(0.4, 0.4, 0.4);

out vec4 Color;

void main() {
	vec3 sampled = texture(CharFont, Tex).rgb;
	if (length(sampled) < 0.05) discard;

	if (Hidden) Color = vec4(HIDDEN_COLOR, 1);
	else 		Color = vec4(sampled, 1);
}
