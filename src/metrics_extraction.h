#ifndef __METRICS_EXTRACTION_H
#define __METRICS_EXTRACTION_H

#include "jpeg_image.h"
#include "rgb2hsv.h"

typedef struct phenology_metrics {
  int *hsv_h;           // The histogram of the H component
  double *SMean;        // The mean S value for each bin of H
  double *VMean;        // The mean V value for each bin of H
  double *SMode;        // The mode S value for each bin of H
  double *VMode;        // The mode V value for each bin of H
  
  int *Gcc;             // The histogram for the Gcc
  rgb *GccMeanColor;      // The mean RGB colors for the Gcc histogram
} phenology_metrics_t;

phenology_metrics_t *calculate_image_metrics(image_t *image);
hsv get_HSV_for_pixel(int pixel, image_t *image);
int get_gcc_bin_for_pixel(int pixel, image_t *image);
double get_gcc_value(unsigned char r, unsigned char g, unsigned char b);
rgb get_rgb_for_pixel(int pixel, image_t *image);

#endif