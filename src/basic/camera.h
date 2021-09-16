#pragma once

#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

#include "gameObject.h"

class Camera : public GameObject {
	public:
		void see(double deltaTime);
		glm::mat4 projectionMatrix;

		void onBindPress(string &bind);
		void onBindRelease(string &bind);

		void setZoomLevel(float zoomLevel);
		float getZoom();
	
	private:
		glm::vec2 position = glm::vec2(0, 0);
		float zoomLevel = 5; // linear value that maps to quadratic
		float minZoomLevel = 2.0;
		float maxZoomLevel = 100.0;
		
		int zoomInRepeating = 0;
		int zoomOutRepeating = 0;
		double zoomInTimer = 0;
		double zoomOutTimer = 0;
};
