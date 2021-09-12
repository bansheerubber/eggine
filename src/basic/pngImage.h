#pragma once

#include <glfw/glfw3.h>
#include <png.h>
#include <string>

using namespace std;

class PNGImage {
	public:
		PNGImage(string fileName);

		png_uint_32 width;
		png_uint_32 height;
		png_byte colorType;
		png_byte bitDepth;
		png_byte* image;

		GLenum getFormat();
		GLenum getType();

	protected:
		string fileName;
};
