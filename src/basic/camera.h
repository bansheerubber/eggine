#pragma once

#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

#include "gameObject.h"

struct CameraBounds {
	double top;
	double right;
	double bottom;
	double left;
};

class Camera : public GameObject {
	public:
		Camera();
		~Camera();

		// ## game_object_definitions Camera
		
		void see(double deltaTime);

		void onBind(std::string &bind, binds::Action action);
		void onAxis(std::string &bind, double value);

		void setZoomLevel(float zoomLevel);
		float getZoom();

		void setPosition(glm::vec2 position);
		glm::vec2 getPosition();

		void pan(glm::vec2 start, glm::vec2 end, double time);

		glm::vec2 mouseToWorld(glm::vec2 mouse);

		CameraBounds getBounds();

		glm::mat4 getProjectionMatrix();
	
	private:
		glm::vec2 position = glm::vec2(0, 0);
		float zoomLevel = 3; // linear value that maps to quadratic
		float minZoomLevel = 2.0;
		float maxZoomLevel = 100.0;

		// keep track of view area bounds
		double top = 0, right = 0, bottom = 0, left = 0;
		
		glm::mat4 projectionMatrix;

		struct {
			int zoomInRepeating = 0;
			int zoomOutRepeating = 0;
			double zoomInTimer = 0;
			double zoomOutTimer = 0;
			bool up = false;
			bool down = false;
			bool left = false;
			bool right = false;
			float xAxis = 0;
			float yAxis = 0;
			float zoomAxis = 0;
		} keyMapping;

		struct {
			glm::vec2 start;
			glm::vec2 end;
			double elapsed;
			double time;
			bool enabled = false;
		} interpolation;

		glm::vec2 getViewport();
};

namespace es {
	void defineCamera();
	esEntryPtr getActiveCamera(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Camera__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Camera__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Camera__pan(esEnginePtr esEngine, unsigned int argc, esEntry* args);
}
