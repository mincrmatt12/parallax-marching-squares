#include <gl3w/GL/gl3w.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "noisegen.h"
#include "renderman.h"
#include "character.h"

const inline float ZOOM = 13;
const inline float SPEED = 0.24;
const inline int LAYERS = 8;
const inline int SEED = 19;
const inline int SIZE = 768;
constexpr inline float SENS = 0.008;

int main(int argc, char ** argv) {
	// Start by creating a window.
	
	sf::ContextSettings c;
	c.attributeFlags = sf::ContextSettings::Core;
	c.depthBits = 24;
	c.stencilBits = 8;
	c.majorVersion = 4;
	c.minorVersion = 5;
	c.antialiasingLevel = 2;

	sf::Window window(sf::VideoMode(1024, 768), "Parallax ASCII Marching Squares", sf::Style::Default, c);
	window.setFramerateLimit(60);

	gl3wInit();

	glViewport(0, 0, 1024, 768);
	glClearColor(0, 0, 0, 1);

	// Generate noisemap
	noisegen::NoiseSet noiseMap(SIZE * SIZE * LAYERS);

	// Create textures
	int noiseTex = noisegen::generateNoiseTexture(SIZE, SIZE, LAYERS, SEED, noiseMap.data);
	int colorTex = noisegen::generateNoiseTexture(SIZE / 4, SIZE / 4, LAYERS, SEED + 2);
	noisegen::DeletingTextureRef _noise_deleter(noiseTex); // |
	noisegen::DeletingTextureRef _color_deleter(colorTex); // |-> use RAII to make sure we erase the thing

	// Load shader
	renderman::Shader shader;

	shader.attach(GL_VERTEX_SHADER, "assets/layer.vert");
	shader.attach(GL_FRAGMENT_SHADER, "assets/layer.frag");
	shader.compile();

	// Create VAO
	renderman::LayerVAO layervao(SIZE, SIZE, LAYERS);

	// Setup camera
	glm::mat4 projection = glm::perspective(glm::radians(100.0), 1024.0 / 768.0, 0.2, LAYERS * 2.0);
	glm::mat4 view       = glm::lookAt(glm::vec3(100, 100, 0), glm::vec3(100, 100, -10), glm::vec3(0, 1, 0));

	glm::vec4 velocity(0);

	// Load font atlas
	sf::Texture fnt;
	fnt.loadFromFile("assets/fnt.png");

	glEnable(GL_DEPTH_TEST);

	// Create character
	character::Char character(SIZE, SIZE, LAYERS, 2.5, noiseMap.data);

	while (true) {
		// Handle events
		{
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type) {
					case sf::Event::Closed:
						goto die;
					case sf::Event::Resized:
						glViewport(0, 0, event.size.width, event.size.height);
						projection = glm::perspective(glm::radians(100.0), (double)event.size.width / event.size.height, 0.2, 300.0);
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape) goto die;
						if (event.key.code == sf::Keyboard::Space) character.jump();
						break;
					default:
						break;
				}
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			velocity.x = -SPEED;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			velocity.x = SPEED;
		}
		else {
			velocity.x = 0;
		}

		// Do rendering
		view = glm::lookAt(character.get_center() + glm::vec3(0, 0.5, ZOOM), character.get_center(), glm::vec3(0, 1, 0));
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		// Plumb shader

		glm::mat4 matrix = projection * view;
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(matrix));
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, noiseTex);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, fnt.getNativeHandle());
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, colorTex);

		layervao.draw();

		character.draw(matrix);
		character.set_x_impulse(velocity.x);
		character.update_physics();

		window.display();
	}

die:
	return 0;
}
