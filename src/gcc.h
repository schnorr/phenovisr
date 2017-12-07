#ifndef __GCC_H
#define __GCC_H

#include <math.h>
#include <strings.h>
#include "jpeg_image.h"
#include "metrics.h"

int gcc_get_bin_for_pixel (PGAMetricType type, int grain, int i, image_t *image);
int *gcc_get_metric (int grain, PGAMetricType type, image_t *image);

#endif