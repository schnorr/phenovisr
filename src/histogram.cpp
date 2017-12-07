#include "histogram.h"

int get_bin (PGAMetricType type, int grain, int i, image_t *image)
{
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[i+0];
  g = iimage[i+1];
  b = iimage[i+2];
  if (is_black(r, g, b)) return -1;
  float value = get_average (type, r, g, b) * grain;
  if (value >= grain) value = grain - 1;
  return floor(value);
}

int *get_metric(int grain, PGAMetricType type, image_t *image)
{
  int *ret = (int*)malloc(grain * sizeof(int));
  bzero(ret, grain*sizeof(int));
  for (int i = 0; i < image->size; i = i+3){
    int x = get_bin (type, grain, i, image);
    if (x >= 0) ret[x]++;
  }  
  return ret;
}