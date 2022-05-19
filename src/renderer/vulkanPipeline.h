#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "../util/hash.h"
#include "primitive.h"
#include "stencil.h"

namespace render {
	struct VulkanPipelineResult {
		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
	};
	
	struct VulkanPipeline { // used for caching a vk pipeline based on commonly used parameters
		class Window* window;
		PrimitiveType topology;
		float viewportWidth;
		float viewportHeight;
		bool depthEnabled;
		bool stencilEnabled;

		uint32_t stencilReference;
		uint32_t stencilCompare;
		uint32_t stencilWriteMask;

		StencilFunction stencilFunction;
		StencilOperation stencilFail;
		StencilOperation depthFail;
		StencilOperation stencilPass;

		class Program* program;
		class VertexAttributes* attributes;

		VulkanPipelineResult newPipeline();
	};

	inline bool operator==(const VulkanPipeline &lhs, const VulkanPipeline &rhs) {
		return lhs.topology == rhs.topology
			&& lhs.viewportWidth == rhs.viewportWidth
			&& lhs.viewportHeight == rhs.viewportHeight
			&& lhs.depthEnabled == rhs.depthEnabled
			&& lhs.stencilEnabled == rhs.stencilEnabled
			&& lhs.stencilReference == rhs.stencilReference
			&& lhs.stencilCompare == rhs.stencilCompare
			&& lhs.stencilWriteMask == rhs.stencilWriteMask
			&& lhs.stencilFunction == rhs.stencilFunction
			&& lhs.stencilFail == rhs.stencilFail
			&& lhs.depthFail == rhs.depthFail
			&& lhs.stencilPass == rhs.stencilPass
			&& lhs.program == rhs.program
			&& lhs.attributes == rhs.attributes;
	}

	inline bool operator!=(const VulkanPipeline &lhs, const VulkanPipeline &rhs) {
		return !(lhs == rhs);
	}
};

namespace std {
	template<>
	struct hash<render::VulkanPipeline> {
		size_t operator()(const render::VulkanPipeline &source) const noexcept {
			uint64_t result = hash<render::PrimitiveType>{}(source.topology);
			result = combineHash(result, source.viewportWidth);
			result = combineHash(result, source.viewportHeight);

			result = combineHash(result, source.depthEnabled);
			result = combineHash(result, source.stencilEnabled);
			result = combineHash(result, source.stencilReference);
			result = combineHash(result, source.stencilCompare);
			result = combineHash(result, source.stencilWriteMask);
			result = combineHash(result, source.stencilFunction);
			result = combineHash(result, source.stencilFail);
			result = combineHash(result, source.depthFail);
			result = combineHash(result, source.stencilPass);

			result = combineHash(result, source.program);
			result = combineHash(result, source.attributes);
			return result;
    }
	};
};
#endif
