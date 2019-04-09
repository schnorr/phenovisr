#ifndef __METRICS_EXTRACTION_H
#define __METRICS_EXTRACTION_H

#include "jpeg_image.h"

typedef struct phenology_metrics {
  int *hsv_h;           // The histogram of the H component
  double *SMean;        // The mean S value for each bin of H
  double *VMean;        // The mean V value for each bin of H
  double *SMode;        // The mode S value for each bin of H
  double *VMode;        // The mode V value for each bin of H
} phenology_metrics_t;

phenology_metrics_t *calculate_image_metrics(image_t *image);

#endif