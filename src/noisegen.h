#ifndef NOISEGEN_H
#define NOISEGEN_H

#include <FastNoiseSIMD.h>
#include <SFML/Graphics/Texture.hpp>
#include <gl3w/GL/gl3w.h>

namespace noisegen {
	struct DeletingTextureRef {
		DeletingTextureRef(GLuint id) : id(id) {}
		~DeletingTextureRef() {glDeleteTextures(1, &id);}
	private:
		GLuint id;
	};

	struct NoiseSet {
		NoiseSet(int size);
		~NoiseSet();

		float * data;
	};

	// Optional buffer gets filled in OPENGL format, i.e. z increases slowest
	GLuint generateNoiseTexture(int width, int height, int layers, int seed, float * buffer = nullptr); 
}

#endif /* ifndef NOISEGEN_H */
