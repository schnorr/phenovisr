#include "metrics.h"

int is_black (rgb RGB) {
  return !(RGB.r + RGB.g + RGB.b);
}

rgb get_rgb_for_pixel(int pixel, image_t *image) {
  unsigned char *iimage = image->image;
  unsigned char r = iimage[pixel];
  unsigned char g = iimage[pixel + 1];
  unsigned char b = iimage[pixel + 2];

  rgb RGB = {(double)r / 255, (double)g / 255, (double)b / 255};
  return RGB;
}

hsv get_HSV_for_pixel(int pixel, image_t *image) {
  rgb RGB = get_rgb_for_pixel(pixel, image);
  if (is_black(RGB)) {
    return {0, 0, 0};
  }

  hsv HSV = rgb2hsv(RGB);
  return HSV;
}

int get_gcc_bin_for_pixel(int pixel, image_t *image) {
  rgb RGB = get_rgb_for_pixel(pixel, image);
  int gccBin;
  if (is_black(RGB)) {
    gccBin = -1;
  }
  else {
    gccBin = floor(get_gcc_value(RGB) * 100);
    gccBin = (gccBin >= 100) ? 99 : gccBin;
  }
  return gccBin;
}

double get_gcc_value(rgb RGB) {
  return (RGB.r + RGB.g + RGB.b) != 0
    ? RGB.g / (RGB.r + RGB.g + RGB.b)
    : 0;
}