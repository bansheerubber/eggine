#ifndef __switch__
#include <glad/gl.h>
#endif

#include "texture.h"

#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "window.h"

render::Texture::Texture(Window* window) {
	this->window = window;
}

void render::Texture::setFilters(TextureFilter minFilter, TextureFilter magFilter) {
	this->minFilter = minFilter;
	this->magFilter = magFilter;
}

void render::Texture::setWrap(TextureWrap uWrap, TextureWrap vWrap) {
	this->uWrap = uWrap;
	this->vWrap = vWrap;
}

void render::Texture::loadPNGFromFile(string filename) {
	ifstream file(filename);

	if(file.bad() || file.fail()) {
		printf("failed to open file for png %s\n", filename.c_str());
		file.close();
		this->window->addError();
		return;
	}

	file.seekg(0, file.end);
	unsigned long length = file.tellg();
	file.seekg(0, file.beg);
	char* buffer = new char[length];
	file.read((char*)buffer, length);
	file.close();

	this->loadPNG((unsigned char*)buffer, length);

	delete[] buffer;	
}

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

void render::Texture::Texture::loadPNG(const unsigned char* buffer, unsigned int size) {
	if(png_sig_cmp(buffer, 0, 8)) {
		printf("could not recognize as PNG\n");
		return;
	}

	/* initialize stuff */
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png) {
		printf("could not initialize PNG reader\n");
		return;
	}

	png_infop info = png_create_info_struct(png);
	if(!info) {
		printf("could not load PNG info\n");
		return;
	}

	if(setjmp(png_jmpbuf(png))) {
		printf("error reading PNG info");
		return;
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

	int bytesPerPixel = 0;
	if(colorType == PNG_COLOR_TYPE_RGB) {
		bytesPerPixel = 3;
	}
	else if(colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
		bytesPerPixel = 4;
	}
	else {
		printf("PNG format not supported\n");
		return;
	}

	png_read_update_info(png, info);

	/* read file */
	if(setjmp(png_jmpbuf(png))) {
		printf("could not read PNG image\n");
		return;
	}

	png_byte** image = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(png_uint_32 y = 0; y < height; y++) {
		image[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, image);

	bytesPerPixel = bytesPerPixel;
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

	this->load(
		imageData,
		width * bytesPerPixel * height,
		width,
		height,
		bitDepth,
		colorType == PNG_COLOR_TYPE_RGB ? 3 : 4
	);

	delete[] imageData;
}

void render::Texture::load(
	const unsigned char* buffer,
	unsigned int bufferSize,
	unsigned int width,
	unsigned int height,
	unsigned int bitDepth,
	unsigned int channels
) {
	this->width = width;
	this->height = height;
	this->bitDepth = bitDepth;
	this->channels = channels;
	
	if(this->minFilter == TEXTURE_FILTER_INVALID || this->magFilter == TEXTURE_FILTER_INVALID) {
		printf("invalid texture filters\n");
		return;
	}

	if(this->uWrap == TEXTURE_WRAP_INVALID || this->vWrap == TEXTURE_WRAP_INVALID) {
		printf("invalid texture wraps\n");
		return;
	}

	#ifdef __switch__
	// allocate memory for the image, we will be deallocating this later though
	switch_memory::Piece* memory = this->window->memory.allocate(
		DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached,
		bufferSize,
		DK_IMAGE_LINEAR_STRIDE_ALIGNMENT
	);
	memcpy(memory->cpuAddr(), buffer, bufferSize);

	dk::ImageLayout layout;
	dk::ImageLayoutMaker{this->window->device}
		.setFlags(0)
		.setFormat(DkImageFormat_R8_Unorm)
		.setDimensions(this->width, this->height)
		.initialize(layout);
	
	// // allocate the actual image that we won't get rid of
	this->memory = this->window->memory.allocate(DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image, layout.getSize(), layout.getAlignment());
	this->image.initialize(layout, this->memory->parent->block, this->memory->start);
	this->imageDescriptor.initialize(this->image);

	this->sampler.setFilter(textureFilterToDkFilter(this->minFilter), textureFilterToDkFilter(this->magFilter));
	this->sampler.setWrapMode(textureWrapToDkWrap(this->uWrap), textureWrapToDkWrap(this->vWrap));
	this->samplerDescriptor.initialize(this->sampler);

	dk::ImageView view{this->image};
	this->window->addTexture(memory, view, this->width, this->height);

	memory->deallocate();
	#else
	// load the GL texture
	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		channelsToGLFormat(this->channels),
		this->width,
		this->height,
		0,
		channelsToGLFormat(this->channels),
		bitDepthToGLFormat(this->bitDepth),
		buffer
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapToGLWrap(this->uWrap));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapToGLWrap(this->vWrap));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterToGLFilter(this->minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterToGLFilter(this->magFilter));
	#endif
}

void render::Texture::bind(unsigned int location) {
	#ifdef __switch__
	this->window->bindTexture(location, this);
	#else
	glActiveTexture(GL_TEXTURE0 + location);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	#endif
}
