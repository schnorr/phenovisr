#ifndef __JPEG_IMAGE_H
#define __JPEG_IMAGE_H
#include <Rcpp.h>
#include <jpeglib.h>

typedef enum { Red, Green, Blue, Undef } PGAMetricType;
typedef struct image {
  unsigned char *image;
  int width;
  int height;
  unsigned long size;
  int channels;
}image_t;

image_t *load_jpeg_image (const char *filename);
int apply_mask (image_t *image, image_t *mask);

#endif
