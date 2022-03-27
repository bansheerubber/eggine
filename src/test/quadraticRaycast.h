#pragma once

#include "raycast.h"

namespace es {
	void defineQuadraticRaycast();
	esEntryPtr quadraticRaycast(esEnginePtr engine, unsigned int argc, esEntryPtr args);
	esEntryPtr projectileAngle(esEnginePtr engine, unsigned int argc, esEntryPtr args);
};

class QuadraticRaycastMarcher {
	public:
		QuadraticRaycastMarcher(glm::ivec3 start, glm::ivec3 end, double speed, unsigned int options);

		bool finished();
		glm::ivec3 value(); // intermediate result
		std::vector<RaycastResult> &cast();

		QuadraticRaycastMarcher &operator++();
	
	private:
		glm::ivec3 start;
		glm::ivec3 end;
		glm::vec2 direction;
		glm::vec2 bounds;
		glm::vec2 steps;
		glm::vec2 delta;
		double angle;
		double speed;
		std::vector<RaycastResult> results;
		bool _finished = false;

		glm::ivec2 position2d;
		glm::ivec3 position;
		glm::ivec3 lastPosition = glm::ivec3(-1, -1, -1);
		double lastZ = 0;

		glm::vec3 currentNormal;
		NeighborDirection neighborDirection = INVALID_DIRECTION;
		NeighborDirection oldNeighborDirection = INVALID_DIRECTION;

		tsl::robin_set<glm::ivec3> currentHits;
		unsigned int options = 0;

		void updateNormal();
		void marchPosition();
		bool canFinish();
};

double calculateProjectileAngle(glm::vec3 start, glm::vec3 end, double speed);

std::vector<RaycastResult> quadraticRaycast(glm::ivec3 start, glm::ivec3 end, double speed, unsigned int options);
