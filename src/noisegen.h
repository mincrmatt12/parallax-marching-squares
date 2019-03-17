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

	GLuint generateNoiseTexture(int width, int height, int layers, int seed); 
}

#endif /* ifndef NOISEGEN_H */
