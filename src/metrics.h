#ifndef __METRICS_H
#define __METRICS_H
#include "jpeg_image.h"
#include "rgb2hsv.h"

int is_black (rgb RGB);
double get_gcc_value(rgb RGB);
rgb get_rgb_for_pixel(int pixel, image_t *image);
hsv get_HSV_for_pixel(int pixel, image_t *image);
int get_gcc_bin_for_pixel(int pixel, image_t *image);

#endif
