#version 430 core

layout(location = 0) in vec3 vPos;

layout(location = 0) uniform mat4 Matrix;

out vec3 Pos;

void main() {
    Pos = vPos;
    gl_Position = Matrix * vec4(vPos, 1.0);
}
