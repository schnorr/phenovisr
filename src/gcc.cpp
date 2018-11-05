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
  if (is_black(r, g, b)){
    // If pixel is black, return white HSV
    return {0,0,0};
  }

  double R = (double)r/255;
  double G = (double)g/255;
  double B = (double)b/255;
  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);
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

HSV_Mean_Histogram* get_hsv_mean_histogram(image_t *image, int consideredPixels) {
  size_t histogramSize = 360 * sizeof(HSV_Mean_Histogram_t);
  HSV_Mean_Histogram_t *histogram = (HSV_Mean_Histogram_t*) malloc(histogramSize);

  // Initialize the histograms array
  for (int i=0; i < 360; i++) {
    histogram[i].H = i;
    histogram[i].HCount = 0;
    histogram[i].VMean = 0;
    histogram[i].SMean = 0;
  }

  // For every pixel...
  for (int i = 0; i < image -> size; i = i+3) {
    hsv HSV = get_HSV_for_pixel(i, image);
    int h = floor(HSV.h);
    double s = HSV.s;
    double v = HSV.v;

    histogram[h].HCount = histogram[h].HCount + 1;
    histogram[h].SMean = histogram[h].SMean + s;
    histogram[h].VMean = histogram[h].VMean + v;
  }

  // Calculate SMean and VMean
  for (int i=0; i < 360; i++) {
    histogram[i].VMean = histogram[i].VMean / histogram[i].HCount;
    histogram[i].SMean = histogram[i].SMean / histogram[i].HCount;
  }

  return histogram;
}
