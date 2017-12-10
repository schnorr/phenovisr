#ifndef __METRICS_H
#define __METRICS_H
#include "jpeg_image.h"
#include "rgb2hsv.h"

typedef struct hsv_histogram {
  int H;       // The H coordinate
  int count;   // The count of the given H
  int nsubins; // How many subins of V?
  int *V;      // A vector of nsubins with the counts of V
}hsv_histogram_t;

double get_metric (PGAMetricType type,
		   unsigned char r,
		   unsigned char g,
		   unsigned char b);
int is_black (unsigned char r,
	      unsigned char g,
	      unsigned char b);

#endif
