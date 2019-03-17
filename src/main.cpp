#include <gl3w/GL/gl3w.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "noisegen.h"
#include "renderman.h"

const inline float ZOOM = 20;
const inline float SPEED = 0.3;
const inline int LAYERS = 64;
const inline int SEED = 13;

int main(int argc, char ** argv) {
	// Start by creating a window.
	
	sf::ContextSettings c;
	c.attributeFlags = sf::ContextSettings::Core;
	c.depthBits = 24;
	c.stencilBits = 8;
	c.majorVersion = 4;
	c.minorVersion = 5;
	c.antialiasingLevel = 2;

	sf::Window window(sf::VideoMode(1024, 768), "Parallax ASCII Marching Squares", sf::Style::Titlebar | sf::Style::Close, c);
	window.setFramerateLimit(60);

	gl3wInit();

	glViewport(0, 0, 1024, 768);
	glClearColor(0, 0, 0, 1);

	// Generate noisemap
	int noiseTex = noisegen::generateNoiseTexture(512, 512, LAYERS, SEED);
	int colorTex = noisegen::generateNoiseTexture(128, 128, LAYERS, SEED + 1);
	noisegen::DeletingTextureRef _noise_deleter(noiseTex); // use RAII to make sure we erase the thing
	noisegen::DeletingTextureRef _color_deleter(colorTex); // use RAII to make sure we erase the thing

	// Load shader
	renderman::Shader shader;

	shader.attach(GL_VERTEX_SHADER, "assets/layer.vert");
	shader.attach(GL_FRAGMENT_SHADER, "assets/layer.frag");
	shader.compile();

	// Create VAO
	renderman::LayerVAO layervao(512, 512, LAYERS);

	// Setup camera
	glm::mat4 projection = glm::perspective(glm::radians(100.0), 1024.0 / 768.0, 0.2, 150.0);
	glm::mat4 view       = glm::lookAt(glm::vec3(100, 100, 0), glm::vec3(100, 100, -10), glm::vec3(0, 1, 0));

	glm::vec3 position(100, 100, ZOOM);
	glm::vec3 velocity(0, 0, 0);

	// Load font atlas
	sf::Texture fnt;
	fnt.loadFromFile("assets/fnt.png");

	glEnable(GL_DEPTH_TEST);

	while (true) {
		// Handle events
		{
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type) {
					case sf::Event::Closed:
						goto die;
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
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			velocity.y = -SPEED;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			velocity.y = SPEED;
		}
		else {
			velocity.y = 0;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			velocity.z = -SPEED;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
			velocity.z = SPEED;
		}
		else {
			velocity.z = 0;
		}

		// Do rendering
		position += velocity;
		view = glm::lookAt(position, glm::vec3(position.x, position.y, position.z-10), glm::vec3(0, 1, 0));
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		// Plumb shader

		glm::mat4 matrix = projection * view;
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(matrix));
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, noiseTex);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, fnt.getNativeHandle());
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, colorTex);

		layervao.draw();

		window.display();
	}

die:
	return 0;
}
