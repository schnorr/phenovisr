#ifndef __GCC_H
#define __GCC_H

#include <math.h>
#include <strings.h>
#include "jpeg_image.h"
#include "metrics.h"

int *get_gcc_histogram (int grain, PGAMetricType type, image_t *image);

#endif
