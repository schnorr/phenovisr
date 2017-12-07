#ifndef __GCC_H
#define __GCC_H

#include <math.h>
#include <strings.h>
#include "jpeg_image.h"
#include "metrics.h"

int *get_histogram (PGAMetricType type, int grain, image_t *image);

#endif
