#ifndef RENDERMAN_H
#define RENDERMAN_H

#include <gl3w/GL/gl3w.h>
#include <vector>

namespace renderman {
	// Layer VAO
	struct LayerVAO {
		LayerVAO(int width, int height, int layers); // Creates the entire 2*layers*3 VAO/VBO combo
		~LayerVAO();

		void draw();
	private:
		GLuint id_vao;
		GLuint id_vbo;

		int length;
	};

	struct Shader {
		GLuint program;

		Shader();

		void use();
		void compile();
		void attach(unsigned int shadType, const char * fPath);
	private:
		bool compiled;
		std::vector<GLuint> shaders;
	};
}

#endif /* ifndef RENDERMAN_H */
