#pragma once

#include <glm/vec3.hpp>

class ALCdevice;
class ALCcontext;
namespace sound {
	class Engine {
		public:
			Engine();

			void setPosition(glm::vec3 position);
		
		private:
			ALCdevice* device;
			ALCcontext* context;
	};
}
