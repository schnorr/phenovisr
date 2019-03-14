#include "metrics_extraction.h"
#include "rgb2hsv.h"

hsv get_HSV_for_pixel(int pixel, image_t *image) {
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[pixel + 0];
  g = iimage[pixel + 1];
  b = iimage[pixel + 2];
  if (!(r + g + b))
  {
    // If pixel is black, return white HSV
    return {0, 0, 0};
  }

  double R = (double)r / 255;
  double G = (double)g / 255;
  double B = (double)b / 255;
  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);
  return HSV;
}

phenology_metrics_t *calculate_image_metrics(image_t *image) {
  // Allocate metrics struct
  phenology_metrics_t *metrics = (phenology_metrics_t*) malloc(sizeof(phenology_metrics_t));
  metrics->hsv_h = (int*) malloc(sizeof(int) * 360);
  
  // Initialize metrics
  for (int i=0; i < 360; i++) {
    metrics->hsv_h[i] = 0;
  }

  // Calculate metrics (for each pixel...)
  for(int i=0; i < image->size; i = i + 3) {
    hsv HSV = get_HSV_for_pixel(i, image);
    int h = floor(HSV.h);
    metrics->hsv_h[h]++;
  }

  return metrics;
}