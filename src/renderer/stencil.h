#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#include <switch.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

namespace render {
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

	#ifdef __switch__
	#else
	inline vk::CompareOp stencilToVulkanStencil(StencilFunction type) {
		switch(type) {
			case STENCIL_NEVER: {
				return vk::CompareOp::eNever;
			}

			case STENCIL_LESS: {
				return vk::CompareOp::eLess;
			}

			case STENCIL_LESS_EQUAL: {
				return vk::CompareOp::eLessOrEqual;
			}

			case STENCIL_GREATER: {
				return vk::CompareOp::eGreater;
			}

			case STENCIL_GREATER_EQUAL: {
				return vk::CompareOp::eGreaterOrEqual;
			}

			case STENCIL_EQUAL: {
				return vk::CompareOp::eEqual;
			}

			case STENCIL_NOT_EQUAL: {
				return vk::CompareOp::eNotEqual;
			}

			case STENCIL_ALWAYS: {
				return vk::CompareOp::eAlways;
			}

			default: {
				return vk::CompareOp::eNever;
			}
		}
	}
	
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
	inline vk::StencilOp stencilOPToVulkanStencilOP(StencilOperation type) {
		switch(type) {
			case STENCIL_KEEP: {
				return vk::StencilOp::eKeep;
			}

			case STENCIL_ZERO: {
				return vk::StencilOp::eZero;
			}

			case STENCIL_REPLACE: {
				return vk::StencilOp::eReplace;
			}

			case STENCIL_INCREMENT: {
				return vk::StencilOp::eIncrementAndClamp;
			}

			case STENCIL_INCREMENT_WRAP: {
				return vk::StencilOp::eIncrementAndWrap;
			}

			case STENCIL_DECREMENT: {
				return vk::StencilOp::eDecrementAndClamp;
			}

			case STENCIL_DECREMENT_WRAP: {
				return vk::StencilOp::eDecrementAndWrap;
			}

			case STENCIL_INVERT: {
				return vk::StencilOp::eInvert;
			}

			default: {
				return vk::StencilOp::eKeep;
			}
		}
	}
	
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
};
