#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#include <switch.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

#include <cstdint>
#include <string>
#include <tsl/robin_map.h>

#include "../util/hash.h"
#include "primitive.h"
#include "stencil.h"

namespace render {
	class Program;

	#ifndef __switch__
	struct VulkanPipelineResult {
		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
	};
	#endif
	
	struct SubState {
		StencilFunction stencilFunction;
		uint32_t stencilReference;
		uint32_t stencilCompare;

		uint32_t stencilWriteMask;

		StencilOperation stencilFail;
		StencilOperation depthFail;
		StencilOperation stencilPass;

		bool stencilEnabled;
		bool depthEnabled;

		float viewportWidth;
		float viewportHeight;

		PrimitiveType primitive;

		class Program* program = nullptr;
		class VertexAttributes* attributes = nullptr;
	};

	inline bool operator==(const SubState &a, const SubState &b) {
		return a.stencilFunction == b.stencilFunction
			&& a.stencilReference == b.stencilReference
			&& a.stencilCompare == b.stencilCompare
			&& a.stencilWriteMask == b.stencilWriteMask
			&& a.stencilFail == b.stencilFail
			&& a.depthFail == b.depthFail
			&& a.stencilPass == b.stencilPass
			&& a.stencilEnabled == b.stencilEnabled
			&& a.depthEnabled == b.depthEnabled
			&& a.primitive == b.primitive
			&& a.program == b.program
			&& a.attributes == b.attributes;
	}

	inline bool operator!=(const SubState &a, const SubState &b) {
		return !(a == b);
	}
};

namespace std {
	template<>
	struct hash<pair<render::Program*, std::string>> {
		size_t operator()(pair<render::Program*, std::string> const& source) const noexcept {
			uint64_t result = hash<std::string>{}(source.second);
			return result ^ ((uint64_t)source.first + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};

	template<>
	struct equal_to<pair<render::Program*, std::string>> {
		bool operator()(const pair<render::Program*, std::string>& x, const pair<render::Program*, std::string>& y) const {
			return x.first == y.first && x.second == y.second;
		}
	};

	template<>
	struct hash<render::SubState> {
		size_t operator()(const render::SubState &source) const noexcept {
			uint64_t result = hash<render::PrimitiveType>{}(source.primitive);
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

namespace render {
	#ifndef __switch__
	struct UniformInfo {
		vk::WriteDescriptorSet write;
		vk::DescriptorBufferInfo bufferInfo;
		vk::DescriptorImageInfo imageInfo;

		class Texture* texture;
		class Piece* uniform;
	};
	#endif

	enum DescriptorSetState {
		DESCRIPTOR_SET_NEEDS_REALLOC,
		DESCRIPTOR_SET_NEEDS_UPDATE,
	};
	
	// render state is built around the concept of a command buffer having commands executed in it that influence the
	// effect of following commands. we need to keep track of this state sometimes, particularly for vulkan, where we
	// may have to create/bind a new render pipeline just to switch shaders, which is a very involved process
	class State {
		friend class Window;
		
		public:
			State();
			State(class Window* window);
			
			#ifdef EGGINE_DEVELOPER_MODE
			void drawImgui();
			#endif

			void draw(PrimitiveType type, unsigned int firstVertex, unsigned int vertexCount, unsigned int firstInstance, unsigned int instanceCount);

			void bindProgram(class Program* program);
			void bindUniform(std::string uniformName, void* data, uint32_t size);
			void bindTexture(std::string uniformName, class Texture* texture);
			void bindVertexAttributes(class VertexAttributes* attributes);

			void setStencilFunction(StencilFunction func, unsigned int reference, unsigned int compare);
			void setStencilMask(unsigned int mask);
			void setStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass);

			void enableStencilTest(bool enable);
			void enableDepthTest(bool enable);

			void reset(); // gets called at the end of the frame

			void resize(unsigned int width, unsigned int height);
		
		private:
			class Window* window = nullptr;

			#ifdef __switch__
			// TODO implement command buffers on state object for deko3d
			#else
			vk::CommandBuffer buffer[2];
			std::vector<UniformInfo> descriptorWrites = std::vector<UniformInfo>(10);
			uint32_t topDescriptor = 0;

			uint32_t descriptorSetIndex = 0;
			DescriptorSetState descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC;

			// static stuff
			static tsl::robin_map<class Program*, uint32_t> DescriptorSetIndex;
			static tsl::robin_map<std::pair<class Program*, std::string>, uint32_t> UniformIndex;
			static tsl::robin_map<SubState, VulkanPipelineResult> VulkanPipelines;

			static void ResetPipelines();

			void updateDescriptorWrites(std::string uniformName);
			#endif
			
			SubState current;
			SubState old;

			void bindPipeline(bool force = false);
	};
};
