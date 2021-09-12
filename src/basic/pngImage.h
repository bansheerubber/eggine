#pragma once

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
		png_byte** image;

	protected:
		string fileName;
};
