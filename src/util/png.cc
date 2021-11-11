#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PNGBuffer {
	const unsigned char* buffer;
	size_t currentIndex;
};

void copyToPNGBuffer(png_structp png, png_bytep output, png_size_t size) {
	PNGBuffer* buffer = (PNGBuffer*)png_get_io_ptr(png);
	if(buffer == NULL) {
		printf("could not load PNG io pointer\n");
		return;
	}

	memcpy(output, &buffer->buffer[buffer->currentIndex], size);
	buffer->currentIndex += size;
}

png loadPng(const unsigned char* buffer, unsigned int size) {
	if(png_sig_cmp(buffer, 0, 8)) {
		printf("could not recognize as PNG\n");
		return {
			buffer: nullptr,
		};
	}

	/* initialize stuff */
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png) {
		printf("could not initialize PNG reader\n");
		return {
			buffer: nullptr,
		};
	}

	png_infop info = png_create_info_struct(png);
	if(!info) {
		printf("could not load PNG info\n");
		return {
			buffer: nullptr,
		};
	}

	if(setjmp(png_jmpbuf(png))) {
		printf("error reading PNG info");
		return {
			buffer: nullptr,
		};
	}

	PNGBuffer* pngBuffer = new PNGBuffer;
	pngBuffer->buffer = buffer;
	pngBuffer->currentIndex = 8;

	png_set_read_fn(png, (void*)pngBuffer, &copyToPNGBuffer);
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);

	png_uint_32 width = png_get_image_width(png, info);
	png_uint_32 height = png_get_image_height(png, info);
	png_byte colorType = png_get_color_type(png, info);
	png_byte bitDepth = png_get_bit_depth(png, info);

	unsigned int bytesPerPixel = 0;
	if(colorType == PNG_COLOR_TYPE_RGB) {
		bytesPerPixel = 3;
	}
	else if(colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
		bytesPerPixel = 4;
	}
	else {
		printf("PNG format not supported\n");
		return {
			buffer: nullptr,
		};
	}

	png_read_update_info(png, info);

	/* read file */
	if(setjmp(png_jmpbuf(png))) {
		printf("could not read PNG image\n");
		return {
			buffer: nullptr,
		};
	}

	png_byte** image = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(png_uint_32 y = 0; y < height; y++) {
		image[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, image);

	png_byte* imageData = new png_byte[width * bytesPerPixel * height];
	png_uint_32 index = 0;
	for(png_uint_32 y = 0; y < height; y++) {
		for(png_uint_32 x = 0; x < width * bytesPerPixel; x++) {
			imageData[y * width * bytesPerPixel + x % (width * bytesPerPixel)] = image[y][x];
		}
	}

	for(png_uint_32 y = 0; y < height; y++) {
		free(image[y]);
	}
	free(image);

	png_destroy_read_struct(&png, &info, NULL);

	return {
		buffer: imageData,
		bufferSize: width * bytesPerPixel * height,
		width: width,
		height: height,
		bitDepth: bitDepth,
		bytesPerPixel: bytesPerPixel,
		colorType == PNG_COLOR_TYPE_RGB ? 3 : 4,
	};
}