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
const inline int LAYERS = 8;
const inline int SEED = 13;
const inline int SIZE = 512;
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
	int noiseTex = noisegen::generateNoiseTexture(SIZE, SIZE, LAYERS, SEED);
	int colorTex = noisegen::generateNoiseTexture(SIZE / 4, SIZE / 4, LAYERS, SEED + 1);
	noisegen::DeletingTextureRef _noise_deleter(noiseTex); // use RAII to make sure we erase the thing
	noisegen::DeletingTextureRef _color_deleter(colorTex); // use RAII to make sure we erase the thing

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

	glm::vec3 position(100, 100, ZOOM);
	glm::vec3 velocity(0, 0, 0);

	// Load font atlas
	sf::Texture fnt;
	fnt.loadFromFile("assets/fnt.png");

	glEnable(GL_DEPTH_TEST);

	// Setup mouse motion
	sf::Vector2i center(512, 384);
	float yaw = 0;
	float pitch = 0;

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
						projection = glm::perspective(glm::radians(100.0), (double)event.size.width / event.size.height, 0.2, 150.0);
						center = sf::Vector2i(event.size.width / 2, event.size.height / 2);
						break;
					case sf::Event::KeyPressed:
						if (event.key.code == sf::Keyboard::Escape) goto die;
						if (event.key.code == sf::Keyboard::F) {pitch = 1.57; yaw = -1.57;}
						break;
					case sf::Event::MouseMoved:
						if (event.mouseMove.x == center.x && event.mouseMove.y == center.y) break;
						{
							// Offset
							sf::Vector2i offset = sf::Vector2i(event.mouseMove.x, event.mouseMove.y) - center;
							sf::Mouse::setPosition(center, window);

							pitch -= (SENS / 2) * offset.y;
							yaw -= SENS * offset.x;

							pitch = glm::clamp(pitch, 0.001f, 3.14f);
						}
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
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
			velocity.z = -SPEED;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			velocity.z = SPEED;
		}
		else {
			velocity.z = 0;
		}

		// Do rendering
		glm::vec3 offset_vec = glm::vec3(
			10 * glm::sin(pitch) * glm::cos(yaw),
			10 * glm::cos(pitch),
			-10 * glm::sin(pitch) * glm::sin(yaw)
		);

		glm::vec3 forward = glm::vec3(-glm::cos(yaw), 0, glm::sin(yaw));
		glm::vec3 left = glm::cross(forward, glm::vec3(0, 1, 0));
		position += forward * velocity.y + left * velocity.x + glm::vec3(0, 1, 0) * velocity.z;

		view = glm::lookAt(position, position - offset_vec, glm::vec3(0, 1, 0));
		
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
