#version 430 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 Texture;

const float SCALE = 0.5; // Smaller than the layer -- makes the motion look better but still voxel-like
const float ADVANCE_X /* naming consistent with layer */ = SCALE / 2.5;
const float ADVANCE_Y                                    = SCALE;
const vec3  ADVANCE = vec3(ADVANCE_X, ADVANCE_Y, 1);

out vec2 Tex; // Mapped in fragment to the texture

layout(location = 0) uniform mat4 Matrix; // Camera
layout(location = 2) uniform vec3 PlayerPos; // Why use a matrix when you could use less memory

void main() {
	gl_Position = Matrix * vec4(Position + PlayerPos, 1.0);
	Tex = Texture;
}
