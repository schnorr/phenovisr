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
  // if (is_black(RGB)) {
  //   return {-1, -1, -1};
  // }

  hsv HSV = rgb2hsv(RGB);
  return HSV;
}

int get_gcc_bin_for_pixel(int pixel, image_t *image) {
  rgb RGB = get_rgb_for_pixel(pixel, image);
  int gccBin;
  if (is_black(RGB)) {
    // This is done to address division by zero. Since I need to consider
    // black pixels, I will assume that for whatever value in which 
    // R, G and B are the same (including a black pixel 0, 0, 0), the Gcc will
    // be 1/3. The limit of x/3x when x approaches 0 is 1/3.
    RGB = {1, 1, 1};
  }
  gccBin = floor(get_gcc_value(RGB) * 100);
  gccBin = (gccBin >= 100) ? 99 : gccBin;
  return gccBin;
}

double get_gcc_value(rgb RGB) {
  return (RGB.r + RGB.g + RGB.b) != 0
    ? RGB.g / (RGB.r + RGB.g + RGB.b)
    : 0;
}