#ifndef __GCC_H
#define __GCC_H

#include <math.h>
#include <strings.h>
#include "jpeg_image.h"
#include "metrics.h"

int *get_histogram (PGAMetricType type, int grain, image_t *image);
hsv_histogram_t *get_HSV_double_histogram (PGAMetricType type, image_t *image, int nbins, int nsubins);
HSV_Mean_Histogram *get_hsv_mean_histogram(image_t *image, int consideredPixels);

#endif
