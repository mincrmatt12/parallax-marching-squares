#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>
#include <vector>
#include <tuple>

class AABB {
	public:
		AABB() = default;
		AABB(glm::vec3 pos, glm::vec3 size);
		AABB(const AABB &other);

		static AABB fromMinMax(glm::vec3 min, glm::vec3 max);
		static AABB encompass(AABB &a, AABB &b);

		void moveTo(glm::vec3 pos);
		void moveTo(double x, double y, double z) { this->moveTo(glm::vec3(x, y, z)); }

		void offset(glm::vec3 offset);
		void setSize(glm::vec3 size);
		void update(glm::vec3 pos, glm::vec3 size);

		glm::vec3 getMin();
		glm::vec3 getMax();
		glm::vec3 getSize();
		glm::vec3 getMid();
		glm::vec3 getHalf();

		bool intersect(AABB &b);
		bool intersect(glm::vec3 &point);

		void fitToBlock();

		void offset(double x, double y, double z);

		void inflate(glm::vec3 inflation);

	private:
		glm::vec3 pos;
		glm::vec3 siz;

};

const AABB FULL_BLOCK = AABB(glm::vec3(0, 0,0), glm::vec3(1, 1, 1));

void moveAndCollide(std::vector<AABB> AABBs, AABB &entity, glm::vec3 &velocity);
std::tuple<bool, glm::vec3, glm::vec3> collideAABBOverVelocity(AABB &box, AABB &entity, glm::vec3 velocity);

#endif /* ifndef AABB_H */
