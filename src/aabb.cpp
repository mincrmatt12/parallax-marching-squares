#include "aabb.h"
#include <limits>

AABB::AABB(glm::vec3 pos, glm::vec3 size) {
	this->pos = pos;
	this->siz = size;
}

AABB AABB::fromMinMax(glm::vec3 min, glm::vec3 max) {
	if (min.x > max.x) std::swap(min.x, max.x);
	if (min.y > max.y) std::swap(min.y, max.y);
	if (min.z > max.z) std::swap(min.z, max.z);

	glm::vec3 size = max - min;
	return AABB(min, size);
}

void AABB::moveTo(glm::vec3 pos) {
	this->pos = pos;
}

void AABB::offset(glm::vec3 offset) {
	this->pos += offset;
}

void AABB::setSize(glm::vec3 size) {
	this->siz = size;
}

void AABB::update(glm::vec3 pos, glm::vec3 size) {
	this->pos = pos;
	this->siz = size;
}

void AABB::fitToBlock() {
}

void AABB::offset(double x, double y, double z) {
	this->pos.x += x;
	this->pos.y += y;
	this->pos.z += z;
}

glm::vec3 AABB::getMin() {
	return this->pos;
}

glm::vec3 AABB::getMax() {
	return this->pos + this->siz;
}

glm::vec3 AABB::getSize() {
	return this->siz;
}

AABB::AABB(const AABB &other) {
	this->pos = glm::vec3(other.pos);
	this->siz = glm::vec3(other.siz);
};

const float EPSILON = 0.000001f;

bool AABB::intersect(AABB &b) {
	glm::vec3 min = this->getMin();
	glm::vec3 max = this->getMax();
	if (max.x <= b.getMin().x) return false;
	if (min.x >= b.getMax().x) return false;
	if (max.y <= b.getMin().y) return false;
	if (min.y >= b.getMax().y) return false;
	if (max.z <= b.getMin().z) return false;
	return min.z < b.getMax().z;

}

AABB AABB::encompass(AABB &a, AABB &b) {
	glm::vec3 min, max;
	min.x = std::min(a.getMin().x, b.getMin().x);
	min.y = std::min(a.getMin().y, b.getMin().y);
	min.z = std::min(a.getMin().z, b.getMin().z);
	max.x = std::max(a.getMax().x, b.getMax().x);
	max.y = std::max(a.getMax().y, b.getMax().y);
	max.z = std::max(a.getMax().z, b.getMax().z);
	return AABB::fromMinMax(min, max);
}

void AABB::inflate(glm::vec3 inflation) {
	this->pos -= inflation;
	this->siz += inflation * 2.0f;
}

glm::vec3 AABB::getMid() {
	return this->getMin() + (this->getSize() / 2.0f);
}

glm::vec3 AABB::getHalf() {
	return this->getSize() / 2.0f;
}

bool AABB::intersect(glm::vec3 &point) {
	return (
			(this->getMin().x < point.x && point.x < this->getMax().x) &&
			(this->getMin().y < point.y && point.y < this->getMax().y) &&
			(this->getMin().z < point.z && point.z < this->getMax().z)
		   );
}

std::tuple<bool, glm::vec3, glm::vec3> collideAABBOverVelocity(AABB &box, AABB &entity, glm::vec3 velocity) {
	// inflate box for minwowski magics

	AABB inflatedBox = AABB(box);
	inflatedBox.inflate(entity.getHalf()); // add padding for collisions
	glm::vec3 originatingRay = entity.getMid();

	// do x collision
	// check if aligned on other two axes:

	if (inflatedBox.getMin().z < originatingRay.z && originatingRay.z < inflatedBox.getMax().z) {
		if (inflatedBox.getMin().y < originatingRay.y && originatingRay.y < inflatedBox.getMax().y) {

			// ok, it could collide, check whether distance between originating ray and both sides is within sensible timeframe

			float dist = velocity.x > 0 ? inflatedBox.getMin().x - originatingRay.x : originatingRay.x -
				inflatedBox.getMax().x;
			if (dist < abs(velocity.x) && dist >= -abs(velocity.x)) {
				// collision on x axis
				// only one collision will ever occur, so bail early

				// dist - EPSILON is the max distance traveled
				if (velocity.x < 0) {
					dist = -dist;
				}

				// check for step case

				if (box.getSize().y <= 0.5f) {
					// needs to step, too
					return {true, glm::vec3(entity.getMin().x + dist + EPSILON, box.getMax().y, entity.getMin().z),
						glm::vec3(0, 0, velocity.z)};
				}

				// normal intersect

				return {true, glm::vec3(entity.getMin().x + dist, entity.getMin().y, entity.getMin().z),
					glm::vec3(0, velocity.y, velocity.z)};


			}

		}
	}
	// ok, now try for y intersect
	if (inflatedBox.getMin().z < originatingRay.z && originatingRay.z < inflatedBox.getMax().z) {
		if (inflatedBox.getMin().x < originatingRay.x && originatingRay.x < inflatedBox.getMax().x) {

			// ok, it could collide, check whether distance between originating ray and both sides is within sensible timeframe

			float dist = velocity.y > 0 ? inflatedBox.getMin().y - originatingRay.y : originatingRay.y -
				inflatedBox.getMax().y;
			if (dist < abs(velocity.y) && dist >= -abs(velocity.y)) {
				// collision on x axis
				// only one collision will ever occur, so bail early

				// dist - EPSILON is the max distance traveled
				if (velocity.y < 0) {
					dist = -dist;
				}

				return {true, glm::vec3(entity.getMin().x, entity.getMin().y + dist, entity.getMin().z),
					glm::vec3(velocity.x, 0, velocity.z)};

			}

		}
	}
	// ok, only the z left!
	if (inflatedBox.getMin().y < originatingRay.y && originatingRay.y < inflatedBox.getMax().y) {
		if (inflatedBox.getMin().x < originatingRay.x && originatingRay.x < inflatedBox.getMax().x) {

			// ok, it could collide, check whether distance between originating ray and both sides is within sensible timeframe

			float dist = velocity.z > 0 ? inflatedBox.getMin().z - originatingRay.z : originatingRay.z -
				inflatedBox.getMax().z;
			if (dist < abs(velocity.z) && dist >= -abs(velocity.z)) {
				// collision on x axis
				// only one collision will ever occur, so bail early

				// dist - EPSILON is the max distance traveled
				if (velocity.z < 0) {
					dist = -dist;
				}
				if (box.getSize().y <= 0.5f) {
					// needs to step, too
					return {true, glm::vec3(entity.getMin().x, box.getMax().y, entity.getMin().z + dist + EPSILON),
						glm::vec3(velocity.x, 0, 0)};
				}
				return {true, glm::vec3(entity.getMin().x, entity.getMin().y, entity.getMin().z + dist),
					glm::vec3(velocity.x, velocity.y, 0)};

			}

		}
	}

	return {false, glm::vec3(), velocity};
}

void moveAndCollide(std::vector<AABB> AABBs, AABB &entity, glm::vec3 &velocity) {
	bool collided = false;
	glm::vec3 pos, vel = velocity;
	if (abs(velocity.x) < EPSILON) vel.x = 0;
	if (abs(velocity.y) < EPSILON) vel.y = 0;
	if (abs(velocity.z) < EPSILON) vel.z = 0;
	if (abs(vel.x) < EPSILON && abs(vel.y) < EPSILON && abs(vel.z) < EPSILON) {
		return;
	}
	for (AABB aabb : AABBs) {
		std::tie(collided, pos, vel) = collideAABBOverVelocity(aabb, entity, vel);
		if (collided) {
			entity.moveTo(pos);
		}
		if (abs(vel.x) < EPSILON && abs(vel.y) < EPSILON && abs(vel.z) < EPSILON) {
			return;
		}
	}
	entity.offset(vel);

	velocity = vel;
}
