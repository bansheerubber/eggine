#pragma once

#if __switch__
#include <deko3d.hpp>
#include "memory.h"
#else
#include <GLFW/glfw3.h>
#endif

#include <png.h>
#include <string>

using namespace std;

namespace render {
	enum TextureFilter {
		TEXTURE_FILTER_INVALID,
		TEXTURE_FILTER_NEAREST,
		TEXTURE_FILTER_LINEAR,
	};

	enum TextureWrap {
		TEXTURE_WRAP_INVALID,
		TEXTURE_WRAP_REPEAT,
		TEXTURE_WRAP_MIRRORED_REPEAT,
		TEXTURE_WRAP_CLAMP_TO_EDGE,
		TEXTURE_WRAP_CLAMP_TO_BORDER,
		TEXTURE_WRAP_MIRROR_CLAMP_TO_EDGE,
	};

	#ifdef __switch__
	inline DkImageFormat channelsAndBitDepthToDkFormat(unsigned int channels, unsigned int bitDepth) {
		switch(bitDepth) {
			case 8: {
				switch(channels) {
					case 1: {
						return DkImageFormat_R8_Unorm;
					}

					case 2: {
						return DkImageFormat_RG8_Unorm;
					}

					case 3: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}

					case 4: {
						return DkImageFormat_RGBA8_Unorm;
					}
				}
			}

			case 16: {
				switch(channels) {
					case 1: {
						return DkImageFormat_R16_Unorm;
					}

					case 2: {
						return DkImageFormat_RG16_Unorm;
					}

					case 3: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}

					case 4: {
						return DkImageFormat_RGBA16_Unorm;
					}
				}
			}

			case 32: {
				switch(channels) {
					case 1: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}

					case 2: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}

					case 3: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}

					case 4: {
						printf("error: deko3d does not support %u channels and %u bitdepth", channels, bitDepth);
						exit(1);
					}
				}
			}
		}
	}
	
	inline DkFilter textureFilterToDkFilter(TextureFilter type) {
		switch(type) {
			case TEXTURE_FILTER_NEAREST: {
				return DkFilter_Nearest;
			}

			case TEXTURE_FILTER_LINEAR: {
				return DkFilter_Linear;
			}
		}
	}

	inline DkWrapMode textureWrapToDkWrap(TextureWrap wrap) {
		switch(wrap) {
			case TEXTURE_WRAP_REPEAT: {
				return DkWrapMode_Repeat;
			}

			case TEXTURE_WRAP_MIRRORED_REPEAT: {
				return DkWrapMode_MirroredRepeat;
			}

			case TEXTURE_WRAP_CLAMP_TO_EDGE: {
				return DkWrapMode_ClampToEdge;
			}

			case TEXTURE_WRAP_CLAMP_TO_BORDER: {
				return DkWrapMode_ClampToBorder;
			}

			case TEXTURE_WRAP_MIRROR_CLAMP_TO_EDGE: {
				return DkWrapMode_MirrorClampToEdge;
			}
		}
	}
	#else
	inline GLenum channelsToGLFormat(unsigned int channels) {
		switch(channels) {
			case 1: {
				return GL_RED;
			}

			case 2: {
				return GL_RG;
			}

			case 3: {
				return GL_RGB;
			}

			case 4: {
				return GL_RGBA;
			}
		}
	}

	inline GLenum bitDepthToGLFormat(unsigned int bitDepth) {
		switch(bitDepth) {
			case 8: {
				return GL_UNSIGNED_BYTE;
			}

			case 16: {
				return GL_UNSIGNED_SHORT;
			}

			case 32: {
				return GL_UNSIGNED_INT;
			}
		}
	}

	inline GLenum textureFilterToGLFilter(TextureFilter type) {
		switch(type) {
			case TEXTURE_FILTER_NEAREST: {
				return GL_NEAREST;
			}

			case TEXTURE_FILTER_LINEAR: {
				return GL_LINEAR;
			}
		}
	}

	inline GLenum textureWrapToGLWrap(TextureWrap wrap) {
		switch(wrap) {
			case TEXTURE_WRAP_REPEAT: {
				return GL_REPEAT;
			}

			case TEXTURE_WRAP_MIRRORED_REPEAT: {
				return GL_MIRRORED_REPEAT;
			}

			case TEXTURE_WRAP_CLAMP_TO_EDGE: {
				return GL_CLAMP_TO_EDGE;
			}

			case TEXTURE_WRAP_CLAMP_TO_BORDER: {
				return GL_CLAMP_TO_BORDER;
			}

			case TEXTURE_WRAP_MIRROR_CLAMP_TO_EDGE: {
				return GL_MIRROR_CLAMP_TO_EDGE;
			}
		}
	}
	#endif
	
	class Texture {
		friend class Window;
		
		public:
			Texture(class Window* window);

			void setFilters(TextureFilter minFilter, TextureFilter magFilter);
			void setWrap(TextureWrap uWrap, TextureWrap vWrap);

			void loadPNGFromFile(string filename);
			void loadPNG(const unsigned char* buffer, unsigned int size);
			void bind(unsigned int location);
			void load(
				const unsigned char* buffer,
				unsigned int bufferSize,
				unsigned int width,
				unsigned int height,
				unsigned int bitDepth,
				unsigned int channels
			);
		
		protected:
			Window* window;

			unsigned int width;
			unsigned int height;
			unsigned int bitDepth;
			unsigned int channels = 1;

			TextureFilter minFilter;
			TextureFilter magFilter;
			TextureWrap uWrap;
			TextureWrap vWrap;

			#ifdef __switch__
			dk::Image image;
			dk::ImageDescriptor imageDescriptor;
			dk::Sampler sampler;
			dk::SamplerDescriptor samplerDescriptor;
			switch_memory::Piece* memory;

			switch_memory::Piece* imageDescriptorMemory;
			switch_memory::Piece* samplerDescriptorMemory;
			#else
			GLuint texture = GL_INVALID_INDEX;
			#endif
	};
};