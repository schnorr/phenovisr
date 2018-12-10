#ifndef __METRICS_H
#define __METRICS_H
#include "jpeg_image.h"
#include "rgb2hsv.h"

typedef struct gcc_histogram {
  int *gcc;
  rgb *color_histogram;
} gcc_histogram_t;

typedef struct hsv_histogram {
  int H;       // The H coordinate
  int count;   // The count of the given H
  int nsubins; // How many subins of V?
  int *V;      // A vector of nsubins with the counts of V
}hsv_histogram_t;

typedef struct HSV_Mean_Histogram {
  int H;      // The H coordinate
  int HCount; // The numper of pixels in the respective H bin
  double SMean;  // The mean value of S in these pixels
  double VMean;  // The mean value of V in these pixels
} HSV_Mean_Histogram_t;

typedef struct HSV_Mode_Histogram {
  int H; // The H coordinate
  int HCount; // The mumber of pixels in the respective H bin
  int SHistogram[10]; // The distribution of S observations
  int VHistogram[10]; // The distribution of V observations
  double SMode; // The mode of the S observations in these pixels
  double VMode; // The mode of the V observations in these pixels
} HSV_Mode_Histogram_t;

double get_metric (PGAMetricType type,
		   unsigned char r,
		   unsigned char g,
		   unsigned char b);
int is_black (unsigned char r,
	      unsigned char g,
	      unsigned char b);

#endif
