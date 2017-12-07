#ifndef __METRICS_H
#define __METRICS_H
#include "jpeg_image.h"

float get_average (PGAMetricType type,
		   unsigned char r,
		   unsigned char g,
		   unsigned char b);
int is_black (unsigned char r,
	      unsigned char g,
	      unsigned char b);

#endif
