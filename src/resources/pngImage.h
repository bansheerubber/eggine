#pragma once

#include <GLFW/glfw3.h>
#include <png.h>
#include <string>

#include "resourceObject.h"

using namespace std;

namespace resources {
	class PNGImage: public ResourceObject {
		public:
			PNGImage(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);

			png_uint_32 width;
			png_uint_32 height;
			png_byte colorType;
			png_byte bitDepth;

			GLenum getFormat();
			GLenum getType();

			GLuint texture = GL_INVALID_INDEX;

		protected:
			string fileName;
			png_byte* image;
	};
};