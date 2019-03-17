#include "renderman.h"
#include <stdexcept>
#include <SFML/System/FileInputStream.hpp>
#include <iostream>

constexpr inline float LAYER_SPACING = 2.0;

renderman::LayerVAO::LayerVAO(int w, int h, int l) {
	// Create vertices

	float vertices[l*2*3*3] /* 2 tris * 3 vertices * 3 components */ ;
	const int X = 0;
	const int Y = 1;
	const int Z = 2;

	auto at = [&](int layer, int tri, int pos, int xyz) -> float& {return vertices[layer*2*3*3 + tri*3*3 + pos*3 + xyz];};

	for (int layer = 0; layer < l; ++layer) {
		float z = -(layer * LAYER_SPACING);

		for (int idx = 0; idx < 6; ++idx) at(layer, idx / 3, idx % 3, Z) = z;

		at(layer, 0, 0, X) = 0;   at(layer, 1, 0, X) = 0;
		at(layer, 0, 0, Y) = 0;   at(layer, 1, 0, Y) = 0;

		at(layer, 0, 1, X) = w;   at(layer, 1, 1, X) = w;
		at(layer, 0, 1, Y) = 0;   at(layer, 1, 1, Y) = h;

		at(layer, 0, 2, X) = w;   at(layer, 1, 2, X) = 0;
		at(layer, 0, 2, Y) = h;   at(layer, 1, 2, Y) = h;
	}

	// Create VBO & VAO
	glGenBuffers(1, &id_vbo);
	glGenVertexArrays(1, &id_vao);

	// Fill VBO
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Setup VAO
	glBindVertexArray(id_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	this->length = l*2*3;
}

renderman::LayerVAO::~LayerVAO() {
	glDeleteVertexArrays(1, &id_vao);
	glDeleteBuffers(1, &id_vao);
}

void renderman::LayerVAO::draw() {
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, this->length);
}

void renderman::Shader::attach(unsigned int shadType, const char * fPath) {
	sf::FileInputStream fis; fis.open(fPath);

	GLsizei siz = fis.getSize();
	GLchar glData[siz+1];
	glData[siz] = 0;
	fis.read(&glData, siz);

	GLuint shader = glCreateShader(shadType);

	const GLchar * glData2 = glData;
	glShaderSource(shader, 1, &glData2, nullptr);
	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(shader, 1024, &log_length, message);
		std::string message2(message, (unsigned long long int) log_length);
		throw std::runtime_error("Compiling failure on shader, log follows:\n" + message2);
	}

	this->shaders.push_back(shader);
}

renderman::Shader::Shader() {
	this->compiled = false;
}

void renderman::Shader::use() {
	glUseProgram(this->program);
}

void renderman::Shader::compile() {
	this->program = glCreateProgram();
	for (auto it = this->shaders.begin(); it != this->shaders.end(); it++) {
		glAttachShader(this->program, *it);
	}
	glLinkProgram(program);
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(program, 1024, &log_length, message);
		std::string message2(message, (unsigned long long int) log_length);
		throw std::runtime_error("Shader link failed, log follows:\n" + message2);
	}

	this->compiled = true;
}
