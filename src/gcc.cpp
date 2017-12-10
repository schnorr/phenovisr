#include "gcc.h"

static int gcc_get_bin_for_pixel (PGAMetricType type, int grain, int i, image_t *image)
{
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[i+0];
  g = iimage[i+1];
  b = iimage[i+2];
  if (is_black(r, g, b)) return -1;
  double value = get_metric (type, r, g, b) * grain;
  if (value >= grain) value = grain - 1;
  return floor(value);
}

int *get_histogram (PGAMetricType type, int grain, image_t *image)
{
  int *ret = (int*)malloc(grain * sizeof(int));
  bzero(ret, grain*sizeof(int));
  for (int i = 0; i < image->size; i = i+3){
    int x = gcc_get_bin_for_pixel (type, grain, i, image);
    if (x >= 0) ret[x]++;
  }  
  return ret;
}

static hsv get_HSV_for_pixel (int pixel, image_t *image)
{
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[pixel+0];
  g = iimage[pixel+1];
  b = iimage[pixel+2];

  double R = (double)r/255;
  double G = (double)g/255;
  double B = (double)b/255;
  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);

  static int z = 100;
  if (z > 0 && HSV.h != 0){
    z--;
//    std::cout << HSV.h << " " << floor(HSV.h) << " " << HSV.s << " " << HSV.v << std::endl;
  }
  return HSV;
}

hsv_histogram_t *get_HSV_double_histogram (PGAMetricType type, image_t *image, int nbins, int nsubins)
{
  size_t size = nbins * sizeof(hsv_histogram_t);
  hsv_histogram_t *ret = (hsv_histogram_t*)malloc(size);
  for(int i = 0; i < nbins; i++){
    ret[i].H = i;
    ret[i].count = 0;
    ret[i].V = (int*)malloc(nsubins * sizeof(int));
    for (int j = 0; j < nsubins; j++){
      ret[i].V[j] = 0;
    }
    ret[i].nsubins = nsubins;
  }

  //for every pixel
  for (int i = 0; i < image->size; i = i+3){
    hsv HSV = get_HSV_for_pixel(i, image);
    int H = floor(HSV.h);
    int V = HSV.v == 1 ? nsubins-1 : floor(HSV.v * nsubins);
    ret[H].H = H;
    ret[H].count = ret[H].count + 1;
    ret[H].V[V] = ret[H].V[V] + 1;
  }
  return ret;
}
