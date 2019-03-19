#include "character.h"
#include "aabb.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <random>

constexpr inline float THRESH = 0.2;
constexpr inline float CHAR_SIZE_X = 3.5 / 2.5;
constexpr inline float CHAR_SIZE_Y = 3.5;
constexpr inline float COLLISION_X = 3.5 / 2.5;
constexpr inline float COLLISION_Y = 3.5;
constexpr inline float GRAVITY = 0.006;
extern const int SIZE;
extern const int LAYERS;

bool character::Char::sample_at(float x, float y, float z) {
	z = glm::floor(z); // use nearest z

	if (x < 0 || x > voxel_size.x) return false;
	if (y < 0 || y > voxel_size.y) return false;
	if (z < 0 || z > voxel_size.z) return false;

	auto raw_sample = [&](int x, int y, int z){
		return this->voxel_data[z * (voxel_size.x * voxel_size.y) + y * (voxel_size.x) + x];
	};

	float c00 = raw_sample(glm::floor(x), glm::floor(y), z);
	float c01 = raw_sample(glm::floor(x), glm::ceil(y), z);
	float c10 = raw_sample(glm::ceil(x), glm::floor(y), z);
	float c11 = raw_sample(glm::ceil(x), glm::ceil(y), z);

	return glm::mix(glm::mix(c00, c01, x - glm::floor(x)), 
					glm::mix(c10, c11, x - glm::floor(x)), y - glm::floor(y)) > THRESH;
}

int character::Char::sample_box(float x, float y, float z) {
	return (sample_at(x, y + CHAR_SIZE_Y, z) << 0) +
		   (sample_at(x + CHAR_SIZE_X, y + CHAR_SIZE_Y, z) << 1) +
		   (sample_at(x, y, z) << 2) +
		   (sample_at(x + CHAR_SIZE_X, y, z) << 3);
}

template<typename Cap>
glm::vec3 pick_position_for(Cap&& sample_at) {
	glm::vec3 position;
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0, SIZE);
		std::uniform_real_distribution<float> dis2(0, LAYERS);

		position.x = dis(gen);
		position.z = glm::floor(dis2(gen));
	}

	bool valid = false;

	for (int Y = 0; Y < 512; ++Y) {
		if (!valid && sample_at(position.x, Y, position.z)) valid = true;
		else if (valid && sample_at(position.x, Y, position.z) && !sample_at(position.x, Y + 3, position.z)) {
			position.y = Y+0.6;
			break;
		}
		else continue;
	}

	return position;
}


character::Char::Char(int sX, int sY, int sZ, float * data) :
	position(0, 0, 0), voxel_size(sX, sY, sZ), voxel_data(data) {
	// Compute a valid location by sampling up in 4s.
	position = pick_position_for([&](auto x, auto y, auto z){return sample_at(x, y, z);});

	shader.attach(GL_VERTEX_SHADER, "assets/player.vert");
	shader.attach(GL_FRAGMENT_SHADER, "assets/player.frag");
	shader.compile();

	// Create a VAO
	float z = 0;
	
	float vertex_data[] = {
		0, 0, z,  0, 0,
		CHAR_SIZE_X, 0, z,  1, 0,
		CHAR_SIZE_X, CHAR_SIZE_Y, z,  1, 1,

		0, 0, z,  0, 0,
		CHAR_SIZE_X, CHAR_SIZE_Y, z,  1, 1,
		0, CHAR_SIZE_Y, z,  0, 1,
	};
	
	glGenBuffers(1, &id_vbo);
	glGenVertexArrays(1, &id_vao);

	// Upload data to buffer
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

	// Setup VAO
	glBindVertexArray(id_vao);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void *)(sizeof(float) * 3));

	character.loadFromFile("assets/char.png");
}

void character::Char::draw(const glm::mat4& matrix) {
	shader.use();
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, character.getNativeHandle());
	glBindVertexArray(id_vao);

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(matrix));
	glm::vec3 real_pos = glm::vec3(position.x, position.y, -position.z * 2);
	glUniform3fv(2, 1, glm::value_ptr(real_pos));

	// Prepare for behind pass
	glDepthFunc(GL_LESS);
	glUniform1i(1, false);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Prepare for front pass
	glDepthFunc(GL_GREATER);
	glUniform1i(1, true);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Cleanup depth state
	glDepthFunc(GL_LESS);
}

void character::Char::jump() {
	if (!inAir)
	{
		velocity.y = 1; 
		inAir = true;
	}
}

void character::Char::set_x_impulse(float x_impulse) {velocity.x = x_impulse;}

void character::Char::update_physics() {
	// Create AABB
	
	AABB mine(position, glm::vec3(CHAR_SIZE_X, CHAR_SIZE_Y, 0.2));

	AABB range = AABB::fromMinMax(position, position + glm::vec3(velocity, 0));
	range.inflate(mine.getHalf());
	range.inflate(glm::vec3{CHAR_SIZE_Y*2});

	// Quantize
	
	range.moveTo(glm::floor(range.getMin() / glm::vec3(CHAR_SIZE_X, CHAR_SIZE_Y, 1)) * glm::vec3(CHAR_SIZE_X, CHAR_SIZE_Y, 1));

	std::vector<AABB> aabbs;

	auto add_aabb = [&](float x, float y) {
		aabbs.emplace_back(glm::vec3{x, y, position.z-0.1}, glm::vec3{CHAR_SIZE_X, CHAR_SIZE_Y, 1});
	};

	for (float x = range.getMin().x; x <= range.getMax().x; x += CHAR_SIZE_X) {
		for (float y = range.getMin().y; y <= range.getMax().y; y += CHAR_SIZE_Y) {
			switch (sample_box(x, y, position.z)) {
				case 0b1111:
					add_aabb(x, y);
				default:
					break;
			}
		}
	}

	// Move
	
	glm::vec3 v{velocity, 0};
	moveAndCollide(aabbs, mine, v);
	velocity = glm::vec2(v.x, v.y);

	// Set our position
	
	position = mine.getMin();

	// Apply gravity
	mine.offset(glm::vec3(0, -0.01, 0));
	
	if (std::none_of(aabbs.begin(), aabbs.end(), [&](auto & aabb){
		return mine.intersect(aabb);
	}))
	{	
		velocity.y -= GRAVITY;
		inAir = true;
	}
	else inAir = false;

	if (position.y < -8) {
		// Compute a valid location by sampling up in 4s.
		respawn();
	}
}

glm::vec3 character::Char::get_center() {
	return position + glm::vec3(CHAR_SIZE_X / 2, CHAR_SIZE_Y / 2, 0);
}

void character::Char::respawn() {
	position = pick_position_for([&](auto x, auto y, auto z){return sample_at(x, y, z);});
}

character::Char::~Char() {
	glDeleteBuffers(1, &id_vbo);
	glDeleteVertexArrays(1, &id_vao);
}
