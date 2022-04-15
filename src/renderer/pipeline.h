#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#include <switch.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

#include "../util/hash.h"
#include "primitive.h"

namespace render {
	struct VulkanPipeline { // used for caching a vk pipeline based on commonly used parameters
		class Window* window;
		PrimitiveType topology;
		float viewportWidth;
		float viewportHeight;

		vk::Pipeline* newPipeline();

		bool operator==(VulkanPipeline const &other) {
			return this->topology == other.topology && this->viewportWidth == other.viewportWidth && this->viewportHeight == other.viewportHeight;
		}
	};
};

namespace std {
	template<>
	struct hash<render::VulkanPipeline> {
		size_t operator()(render::VulkanPipeline const &source) const noexcept {
			uint64_t result = hash<render::PrimitiveType>{}(source.topology);
			result = combineHash(result, source.viewportWidth);
			result = combineHash(result, source.viewportHeight);
			return result;
    }
	};
};
