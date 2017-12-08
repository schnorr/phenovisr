#ifndef __METRICS_H
#define __METRICS_H
#include "jpeg_image.h"
#include "rgb2hsv.h"

double get_metric (PGAMetricType type,
		   unsigned char r,
		   unsigned char g,
		   unsigned char b);
int is_black (unsigned char r,
	      unsigned char g,
	      unsigned char b);

#endif
