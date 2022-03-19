#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#include <switch.h>
#else
#include <GLFW/glfw3.h>
#endif

#include "../engine/developer.h"

#include <litehtml.h>
#include <glm/vec4.hpp>
#include <vector>

#include "../util/crop.h"
#include "../renderer/litehtmlContainer.h"
#include "memory.h"
#include "../util/png.h"
#include "../renderer/texture.h"
#include "../util/time.h"

namespace render {
	enum PrimitiveType {
		PRIMITIVE_POINTS,
		PRIMITIVE_LINES,
		PRIMITIVE_LINE_LOOP,
		PRIMITIVE_LINE_STRIP,
		PRIMITIVE_LINES_ADJACENCY,
		PRIMITIVE_LINE_STRIP_ADJACENCY,
		PRIMITIVE_TRIANGLES,
		PRIMITIVE_TRIANGLE_STRIP,
		PRIMITIVE_TRIANGLE_FAN,
		PRIMITIVE_TRIANGLES_ADJACENCY,
		PRIMITIVE_TRIANGLE_STRIP_ADJACENCY,
		PRIMITIVE_PATCHES,
	};

	#ifdef __switch__
	#define COMMAND_BUFFER_SLICE_COUNT 2
	#define IMAGE_SAMPLER_DESCRIPTOR_COUNT 32

	inline DkPrimitive primitiveToDkPrimitive(PrimitiveType type) {
		switch(type) {
			default:
			case PRIMITIVE_POINTS: {
				return DkPrimitive_Points;
			}

			case PRIMITIVE_LINES: {
				return DkPrimitive_Lines;
			}

			case PRIMITIVE_LINE_LOOP: {
				return DkPrimitive_LineLoop;
			}

			case PRIMITIVE_LINE_STRIP: {
				return DkPrimitive_LineStrip;
			}

			case PRIMITIVE_LINES_ADJACENCY: {
				return DkPrimitive_LinesAdjacency;
			}

			case PRIMITIVE_LINE_STRIP_ADJACENCY: {
				return DkPrimitive_LineStripAdjacency;
			}

			case PRIMITIVE_TRIANGLES: {
				return DkPrimitive_Triangles;
			}

			case PRIMITIVE_TRIANGLE_STRIP: {
				return DkPrimitive_TriangleStrip;
			}

			case PRIMITIVE_TRIANGLE_FAN: {
				return DkPrimitive_TriangleFan;
			}

			case PRIMITIVE_TRIANGLES_ADJACENCY: {
				return DkPrimitive_TrianglesAdjacency;
			}

			case PRIMITIVE_TRIANGLE_STRIP_ADJACENCY: {
				return DkPrimitive_TriangleStripAdjacency;
			}

			case PRIMITIVE_PATCHES: {
				return DkPrimitive_Patches;
			}
		}
	}
	#else
	inline GLenum primitiveToGLPrimitive(PrimitiveType type) {
		switch(type) {
			case PRIMITIVE_POINTS: {
				return GL_POINTS;
			}

			case PRIMITIVE_LINES: {
				return GL_LINES;
			}

			case PRIMITIVE_LINE_LOOP: {
				return GL_LINE_LOOP;
			}

			case PRIMITIVE_LINE_STRIP: {
				return GL_LINE_STRIP;
			}

			case PRIMITIVE_LINES_ADJACENCY: {
				return GL_LINES_ADJACENCY;
			}

			case PRIMITIVE_LINE_STRIP_ADJACENCY: {
				return GL_LINE_STRIP_ADJACENCY;
			}

			case PRIMITIVE_TRIANGLES: {
				return GL_TRIANGLES;
			}

			case PRIMITIVE_TRIANGLE_STRIP: {
				return GL_TRIANGLE_STRIP;
			}

			case PRIMITIVE_TRIANGLE_FAN: {
				return GL_TRIANGLE_FAN;
			}

			case PRIMITIVE_TRIANGLES_ADJACENCY: {
				return GL_TRIANGLES_ADJACENCY;
			}

			case PRIMITIVE_TRIANGLE_STRIP_ADJACENCY: {
				return GL_TRIANGLE_STRIP_ADJACENCY;
			}

			case PRIMITIVE_PATCHES: {
				return GL_PATCHES;
			}

			default: {
				return GL_INVALID_ENUM;
			}
		}
	}
	#endif

	enum StencilFunction {
		STENCIL_NEVER,
		STENCIL_LESS,
		STENCIL_LESS_EQUAL,
		STENCIL_GREATER,
		STENCIL_GREATER_EQUAL,
		STENCIL_EQUAL,
		STENCIL_NOT_EQUAL,
		STENCIL_ALWAYS,
	};

	#ifdef __switch
	#else
	inline GLenum stencilToGLStencil(StencilFunction type) {
		switch(type) {
			case STENCIL_NEVER: {
				return GL_NEVER;
			}

			case STENCIL_LESS: {
				return GL_LESS;
			}

			case STENCIL_LESS_EQUAL: {
				return GL_LEQUAL;
			}

			case STENCIL_GREATER: {
				return GL_GREATER;
			}

			case STENCIL_GREATER_EQUAL: {
				return GL_GEQUAL;
			}

			case STENCIL_EQUAL: {
				return GL_EQUAL;
			}

			case STENCIL_NOT_EQUAL: {
				return GL_NOTEQUAL;
			}

			case STENCIL_ALWAYS: {
				return GL_ALWAYS;
			}

			default: {
				return GL_NEVER;
			}
		}
	}
	#endif

	enum StencilOperation {
		STENCIL_KEEP,
		STENCIL_ZERO,
		STENCIL_REPLACE,
		STENCIL_INCREMENT,
		STENCIL_INCREMENT_WRAP,
		STENCIL_DECREMENT,
		STENCIL_DECREMENT_WRAP,
		STENCIL_INVERT,
	};

	#ifdef __switch__
	#else
	inline GLenum stencilOPToGLStencilOP(StencilOperation type) {
		switch(type) {
			case STENCIL_KEEP: {
				return GL_KEEP;
			}

			case STENCIL_ZERO: {
				return GL_ZERO;
			}

			case STENCIL_REPLACE: {
				return GL_REPLACE;
			}

			case STENCIL_INCREMENT: {
				return GL_INCR;
			}

			case STENCIL_INCREMENT_WRAP: {
				return GL_INCR_WRAP;
			}

			case STENCIL_DECREMENT: {
				return GL_DECR;
			}

			case STENCIL_DECREMENT_WRAP: {
				return GL_DECR_WRAP;
			}

			case STENCIL_INVERT: {
				return GL_INVERT;
			}

			default: {
				return GL_KEEP;
			}
		}
	}
	#endif

	// the Window class handles our deko3d front/back buffers as well other global-ish datastructres
	// for opengl, we just handle a GLFW window
	class Window {
		friend LiteHTMLContainer;
		
		public:
			double deltaTime = 0.0;
			unsigned int width = 1280;
			unsigned int height = 720;

			shared_ptr<litehtml::document> htmlDocument = nullptr;
			render::LiteHTMLContainer* htmlContainer = nullptr;
			
			void initialize(); // start the graphics
			void deinitialize(); // end the graphics
			void initializeHTML(); // load index.html
			void resize(unsigned int width, unsigned int height); // resize the window
			void prerender();
			void render();
			void draw(PrimitiveType type, unsigned int firstVertex, unsigned int vertexCount, unsigned int firstInstance, unsigned int instanceCount);
			void addError();
			unsigned int getErrorCount();
			void clearErrors();
			void registerHTMLUpdate();

			void setStencilFunction(StencilFunction func, unsigned int reference, unsigned int mask);
			void setStencilMask(unsigned int mask);
			void setStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass);
			void enableStencilTest(bool enable);

			void enableDepthTest(bool enable);

			#ifdef __switch__
			switch_memory::Manager memory = switch_memory::Manager(this);
			dk::UniqueDevice device;
			dk::CmdBuf commandBuffer;

			HidAnalogStickState leftStick;
			HidAnalogStickState rightStick;
			uint64_t buttons;

			void addTexture(switch_memory::Piece* tempMemory, dk::ImageView& view, unsigned int width, unsigned int height);
			void bindTexture(unsigned int location, class Texture* texture);
			#else
			GLFWwindow* window = nullptr;
			GLFWgamepadstate gamepad;
			bool hasGamepad;
			#endif

			binds::GamepadButtons axisDPadCounters[4] = {binds::INVALID_BUTTON, binds::INVALID_BUTTON, binds::INVALID_BUTTON, binds::INVALID_BUTTON};

		protected:
			unsigned int errorCount = 0;

			glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

			uint64_t lastRenderTime = getMicrosecondsNow();

			litehtml::context htmlContext;
			uint64_t htmlChecksum = 0;
			uint64_t lastHTMLChecksum = 0;
			
			#ifdef __switch__
			switch_memory::Piece* imageDescriptorMemory;
			switch_memory::Piece* samplerDescriptorMemory;
			
			dk::MemBlock commandBufferMemory;
			unsigned int commandBufferSize = 1024 * 1024; // 1 MB
			unsigned int commandBufferCount = COMMAND_BUFFER_SLICE_COUNT;
			unsigned int commandBufferSliceSize = this->commandBufferSize / COMMAND_BUFFER_SLICE_COUNT;
			unsigned int currentCommandBuffer = 0;
			unsigned int signaledFence = 0;
			dk::Fence commandBufferFences[COMMAND_BUFFER_SLICE_COUNT];
			
			unsigned int staticCommandBufferSize = 16 * 1024; // 16 KB
			dk::MemBlock staticCommandBufferMemory;
			dk::CmdBuf staticCommandBuffer; // always inserted at the start of prerender
			DkCmdList staticCommandList;

			dk::UniqueQueue queue;

			dk::MemBlock framebufferMemory;
			dk::Image framebuffers[2]; // front and back buffer
			DkCmdList framebufferCommandLists[2]; // command lists to bind front and back buffers
			dk::Swapchain swapchain; // handles swapping the front/back buffer during the rendering process

			dk::MemBlock textureCommandBufferMemory;
			dk::CmdBuf textureCommandBuffer;
			dk::Fence textureFence;

			// static data used for building static command list
			DkViewport viewport = { 0.0f, 0.0f, (float)this->width, (float)this->height, 0.0f, 1.0f };
			DkScissor scissor = { 0, 0, this->width, this->height };
			dk::RasterizerState rasterizerState = dk::RasterizerState {};
			dk::ColorState colorState = dk::ColorState {};
			dk::ColorWriteState colorWriteState = dk::ColorWriteState {};
			dk::BlendState blendState = dk::BlendState {};

			PadState pad;
			#endif
	};
};
