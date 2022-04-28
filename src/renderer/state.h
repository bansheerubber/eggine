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

#include "primitive.h"
#include "stencil.h"
#include "vulkanPipeline.h"

namespace render {
	class Program;
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
};

namespace render {
	struct SubState {
		StencilFunction stencilFunction;
		uint32_t stencilReference;
		uint32_t stencilMask;

		uint32_t stencilWriteMask;

		StencilOperation stencilFail;
		StencilOperation depthFail;
		StencilOperation stencilPass;

		bool stencilEnabled;
		bool depthEnabled;

		PrimitiveType primitive;

		class Program* program = nullptr;

		class VertexAttributes* attributes = nullptr;
	};

	inline bool operator==(const SubState &a, const SubState &b) {
		return a.stencilFunction == b.stencilFunction
			&& a.stencilReference == b.stencilReference
			&& a.stencilMask == b.stencilMask
			&& a.stencilWriteMask == b.stencilWriteMask
			&& a.stencilFail == b.stencilFail
			&& a.depthFail == b.depthFail
			&& a.stencilPass == b.stencilPass
			&& a.stencilEnabled == b.stencilEnabled
			&& a.depthEnabled == b.depthEnabled
			&& a.primitive == b.primitive			&& a.program == b.program
			&& a.attributes == b.attributes;
	}

	inline bool operator!=(const SubState &a, const SubState &b) {
		return !(a == b);
	}

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
			
			void draw(PrimitiveType type, unsigned int firstVertex, unsigned int vertexCount, unsigned int firstInstance, unsigned int instanceCount);

			void bindProgram(class Program* program);
			void bindUniform(std::string uniformName, void* data, uint32_t size);
			void bindTexture(std::string uniformName, class Texture* texture);
			void bindVertexAttributes(class VertexAttributes* attributes);

			void setStencilFunction(StencilFunction func, unsigned int reference, unsigned int mask);
			void setStencilMask(unsigned int mask);
			void setStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass);
			void enableStencilTest(bool enable);
			void enableDepthTest(bool enable);

			void reset(); // gets called at the end of the frame
		
		private:
			class Window* window = nullptr;

			#ifdef __switch__
			// TODO implement command buffers on state object for deko3d
			#else
			vk::CommandBuffer buffer[2];
			std::vector<UniformInfo> descriptorWrites = std::vector<UniformInfo>(10);
			uint32_t topDescriptor = 0;

			void updateDescriptorWrites(std::string uniformName);
			#endif
			
			SubState current;
			SubState old;

			VulkanPipeline oldPipeline;

			bool applied = false;
			uint32_t descriptorSetIndex = 0;

			DescriptorSetState descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC;

			void bindPipeline();

			// static stuff
			static tsl::robin_map<class Program*, uint32_t> DescriptorSetIndex;
			static tsl::robin_map<std::pair<class Program*, std::string>, uint32_t> UniformIndex;
	};
};
