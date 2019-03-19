#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>
#include <gl3w/GL/gl3w.h>
#include <SFML/Graphics/Texture.hpp>
#include "renderman.h"

namespace character {
	struct Char {
		Char(int sizeX, int sizeY, int sZ, float * voxel);
		~Char();

		void draw(const glm::mat4& matrix); // binds our vao and draws both passes
		void jump();
		void update_physics();
		void set_x_impulse(float x_impulse);
		void respawn();

		glm::vec3 get_center();
	private:
		bool sample_at(float x, float y, float z);
		int  sample_box(float x, float y, float z);

		glm::vec3 position;
		glm::vec2 velocity{0, 0};
		glm::ivec3 voxel_size;
		float * voxel_data;
		renderman::Shader shader;

		bool inAir = false;

		GLuint id_vao, id_vbo;
		sf::Texture character;
	};
}

#endif /* ifndef CHARACTER_H */
