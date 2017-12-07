#ifndef __HISTOGRAM_H
#define __HISTOGRAM_H
#include <math.h>
#include <strings.h>
#include "jpeg_image.h"
#include "metrics.h"

int get_bin (PGAMetricType type, int grain, int i, image_t *image);
int *get_metric (int grain, PGAMetricType type, image_t *image);

#endif
