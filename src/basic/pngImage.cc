#include "pngImage.h"

#include <fstream>

PNGImage::PNGImage(string fileName) {
  this->fileName = fileName;

  png_byte header[8]; // 8 is the maximum size that can be checked

  /* open file and test for it being a png */
  FILE* fp = fopen(fileName.c_str(), "rb");
  fread(header, 1, 8, fp);
  if(png_sig_cmp(header, 0, 8)) {
    printf("could not recognize %s as PNG\n", fileName.c_str());
		fclose(fp);
		return;
	}

  /* initialize stuff */
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if(!png) {
		printf("could not initialize PNG reader\n");
		fclose(fp);
		return;
	}

  png_infop info = png_create_info_struct(png);
  if(!info) {
		printf("could not load PNG info\n");
		fclose(fp);
		return;
	}

  if(setjmp(png_jmpbuf(png))) {
    printf("error reading PNG info");
		fclose(fp);
		return;
	}

  png_init_io(png, fp);
  png_set_sig_bytes(png, 8);

  png_read_info(png, info);

  this->width = png_get_image_width(png, info);
  this->height = png_get_image_height(png, info);
  this->colorType = png_get_color_type(png, info);
  this->bitDepth = png_get_bit_depth(png, info);

  int bytesPerPixel = 0;
  if(this->colorType == PNG_COLOR_TYPE_RGB) {
    bytesPerPixel = 3;
  }
  else if(this->colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
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
  for(png_uint_32 y = 0; y < this->height; y++) {
    image[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

  png_read_image(png, image);

  this->image = new png_byte[this->width * bytesPerPixel * this->height];
  png_uint_32 index = 0;
  for(png_uint_32 y = 0; y < this->height; y++) {
    for(png_uint_32 x = 0; x < this->width * bytesPerPixel; x++) {
      this->image[y * this->width * bytesPerPixel + x % (this->width * bytesPerPixel)] = image[y][x];
    }
  }

  png_destroy_read_struct(&png, &info, NULL);

  fclose(fp);
}

GLenum PNGImage::getFormat() {
  if(this->colorType == PNG_COLOR_TYPE_RGB) {
    return GL_RGB;
  }
  else if(this->colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
    return GL_RGBA;
  }
  else {
    return GL_INVALID_INDEX;
  }
}

GLenum PNGImage::getType() {
  if(this->bitDepth == 8) {
    return GL_UNSIGNED_BYTE;
  }
  else if(this->bitDepth == 16) {
    return GL_UNSIGNED_SHORT;
  }
  else {
    return GL_INVALID_INDEX;
  }
}
