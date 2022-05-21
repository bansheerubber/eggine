#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

#include <unordered_set>
#include <string>

#include "memory.h"

#define RENDER_UNBIND_VERTEX_BUFFERS

namespace render {
	#ifndef __switch__
	enum VertexSubBufferState {
		VERTEX_SUB_BUFFER_IN_USE,
		VERTEX_SUB_BUFFER_DATA_OUT_OF_DATE,
		VERTEX_SUB_BUFFER_SIZE_OUT_OF_DATE,
	};
	
	struct VertexSubBuffer {
		VertexSubBufferState state = VERTEX_SUB_BUFFER_SIZE_OUT_OF_DATE;
		Piece* buffer = nullptr;
	};
	#endif
	
	class VertexBuffer {
		friend class VertexAttributes;
		friend class Window;
		
		public:
			VertexBuffer(class Window* window, std::string debugName = "n/a");
			~VertexBuffer();

			class Window* window = nullptr;

			#ifdef __switch__
			void bind(unsigned short id);
			#else
			void bind();
			#endif

			void setDynamicDraw(bool isDynamicDraw);
			void setSubData(void* data, unsigned int size, unsigned int offset);
			void setData(void* data, unsigned int size, unsigned int align);
			void reallocate();

			#ifndef __switch__
			vk::Buffer getVulkanBuffer();
			vk::Buffer getVulkanDynamicBuffer();
			#endif

			bool isOutOfDateBuffer();

			static void RemoveOutOfDateBuffer(VertexBuffer* buffer);
		
		protected:
			#ifdef __switch__
			bool memoryAllocated = false;
			bool forceReallocate = false;
			Piece* memory = nullptr;
			uint32_t size = 0;
			uint32_t align = 0;
			#else
			GLuint bufferId = GL_INVALID_INDEX;
			GLenum usage = GL_STATIC_DRAW;
			uint32_t size = 0; // current size of the buffer

			Piece* stagingBuffer = nullptr;
			Piece* gpuBuffer = nullptr;

			std::vector<VertexSubBuffer> dynamicBuffers = std::vector<VertexSubBuffer>(2);

			bool isDynamicDraw = false;
			bool dynamicBufferInitialized = false;

			bool needsCopy = false;
			#endif
			uint32_t oldSize = 0;

			void allocateBuffer();
			void destroyBuffer();

			void handleOutOfDateBuffer();

			std::string debugName = "";

			static std::array<std::unordered_set<VertexBuffer*>, 2> OutOfDateBuffers;
	};
};
