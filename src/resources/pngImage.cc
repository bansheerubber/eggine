#include "pngImage.h"

#include <cstring>
#include <fstream>

struct PNGBuffer {
  const unsigned char* buffer;
  size_t currentIndex;
};

void copyDataToPNGBuffer(png_structp png, png_bytep output, png_size_t size) {
  PNGBuffer* buffer = (PNGBuffer*)png_get_io_ptr(png);
  if(buffer == NULL) {
    printf("could not load PNG io pointer\n");
    return;
  }

  memcpy(output, &buffer->buffer[buffer->currentIndex], size);
  buffer->currentIndex += size;
}

resources::PNGImage::PNGImage(
  ResourceManager* manager,
  carton::Metadata* metadata,
  const unsigned char* buffer,
  size_t bufferSize
) : ResourceObject(manager, metadata) {
  if(png_sig_cmp(buffer, 0, 8)) {
    printf("could not recognize %s as PNG\n", fileName.c_str());
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

  png_set_read_fn(png, (void*)pngBuffer, &copyDataToPNGBuffer);
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

  // load the GL texture
  // glGenTextures(1, &this->texture);
  // glBindTexture(GL_TEXTURE_2D, this->texture);

  // glTexImage2D(
  //   GL_TEXTURE_2D,
  //   0,
  //   this->getFormat(),
  //   this->width,
  //   this->height,
  //   0,
  //   this->getFormat(),
  //   this->getType(),
  //   this->image
  // );

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  delete this->image; // delete the image once we're done with it
}

// GLenum resources::PNGImage::getFormat() {
//   if(this->colorType == PNG_COLOR_TYPE_RGB) {
//     return GL_RGB;
//   }
//   else if(this->colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
//     return GL_RGBA;
//   }
//   else {
//     return GL_INVALID_INDEX;
//   }
// }

// GLenum resources::PNGImage::getType() {
//   if(this->bitDepth == 8) {
//     return GL_UNSIGNED_BYTE;
//   }
//   else if(this->bitDepth == 16) {
//     return GL_UNSIGNED_SHORT;
//   }
//   else {
//     return GL_INVALID_INDEX;
//   }
// }
