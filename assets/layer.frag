#version 430 core

in vec3 Pos;

layout(binding = 0) uniform sampler3D NoiseMap;
layout(binding = 1) uniform sampler2D Font;
layout(binding = 2) uniform sampler3D ColorMap;

out vec4 Color;

const float ADVANCE_X = 4 / 2.5;
const float ADVANCE_Y = 4;
const float THRESH = 0.35;
const vec3  ADVANCE = vec3(ADVANCE_X, ADVANCE_Y, 1);
const float PADDING = 2 / 20;
const float LAYER_DISTANCE = 2.0;

// indexes into: right bar, _, overline, / \ left bar

const int[] LOOKUP_TABLE = {
	-1, // never used
	-1,  // x x x d
	-1,  // x x c x
	1,  // x x c d
	-1,  // x b x x
	0,  // x b x d
	3,  // x b c x
	3,  // x b c d
	-1,  // a x x x
	5,  // a x x d
	5,  // a x c x
	4,  // a x c d
	2,  // a b x x
	4,  // a b x d
	3,  // a b c x
	-1, // never used
};

const float IDX_SIZE = 1 / 6.0;

bool sample_at(float x, float y) {
	vec3 round_pos = floor(Pos / ADVANCE) * ADVANCE;
	return texture(NoiseMap, (round_pos + vec3(x, y, 0)) / (textureSize(NoiseMap, 0) * vec3(1, 1, -LAYER_DISTANCE))).r > THRESH;
}

vec3 hsv2rgb(float h) {
	vec3 c = vec3(h, 0.5, 1.0);
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 sample_color(float x, float y) {
	vec3 round_pos = floor(Pos / ADVANCE) * ADVANCE;
	return hsv2rgb(texture(ColorMap, (round_pos + vec3(x, y, 0)) / (textureSize(ColorMap, 0) * vec3(1, 1, -LAYER_DISTANCE))).r);
}

void main() {
	
	// First step: sample all four corners
	bool tl = sample_at(0, 0);
	bool tr = sample_at(ADVANCE_X, 0);
	bool bl = sample_at(0, ADVANCE_Y);
	bool br = sample_at(ADVANCE_X, ADVANCE_Y);
	// (a b c d)
	int index = 1 * int(br) + 2 * int(bl) + 4 * int(tr) + 8 * int(tl);

	// If empty, discard
	if (index == 15) {
		Color = vec4(0, 0, 0, 1);
		return;
	}
	if (LOOKUP_TABLE[index] == -1) discard;


	// Otherwise, lookup through table
	int idx = LOOKUP_TABLE[index];

	// Now, using the FontData get a tex coord on the Font
	vec2 round_pos = mod(Pos.xy, ADVANCE.xy) / ADVANCE.xy;
	round_pos *= vec2(IDX_SIZE, 1);
	round_pos.x += IDX_SIZE * idx;

	vec4 preColor = texture(Font, round_pos.xy);
	if (preColor.r < 0.4) discard;
	Color = preColor * vec4(sample_color(ADVANCE_X / 2, ADVANCE_Y / 2), 1);
}
