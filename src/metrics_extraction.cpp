#include "metrics_extraction.h"
#include "rgb2hsv.h"

#define MODE_SUBINS 10

rgb get_rgb_for_pixel(int pixel, image_t *image) {
  unsigned char *iimage = image->image;
  unsigned char r = iimage[pixel];
  unsigned char g = iimage[pixel + 1];
  unsigned char b = iimage[pixel + 2];

  rgb RGB = {(double)r / 255, (double)g / 255, (double)b / 255};
  return RGB;
}

hsv get_HSV_for_pixel(int pixel, image_t *image) {
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[pixel + 0];
  g = iimage[pixel + 1];
  b = iimage[pixel + 2];
  if (!(r + g + b)) {
    // If pixel is black, return white HSV
    return {0, 0, 0};
  }

  double R = (double)r / 255;
  double G = (double)g / 255;
  double B = (double)b / 255;
  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);
  return HSV;
}

int get_gcc_bin_for_pixel(int pixel, image_t *image) {
  unsigned char *iimage = image->image;
  unsigned char r, g, b;
  r = iimage[pixel + 0];
  g = iimage[pixel + 1];
  b = iimage[pixel + 2];
  int gccBin;
  if (!(r + g + b)) {
  // If pixel is black
    gccBin = -1;
  } else {
    gccBin = floor(get_gcc_value(r, g, b) * 100);
    gccBin = (gccBin >= 100) ? 99 : gccBin;
  }
  return gccBin;
}

double get_gcc_value(unsigned char r, unsigned char g, unsigned char b) {
  return (r + g + b) == 0 ? 0 : (double)g / (double)(r + g + b);
}

phenology_metrics_t *calculate_image_metrics(image_t *image) {
  // Allocate metrics struct
  phenology_metrics_t *metrics = (phenology_metrics_t*) malloc(sizeof(phenology_metrics_t));
  metrics->hsv_h = (int *)malloc(sizeof(int) * 360);
  metrics->SMean = (double *)malloc(sizeof(double) * 360);
  metrics->VMean = (double *)malloc(sizeof(double) * 360);
  metrics->SMode = (double *)malloc(sizeof(double) * 360);
  metrics->VMode = (double *)malloc(sizeof(double) * 360);
  metrics->Gcc = (int *) malloc(sizeof(int) * 100);
  metrics->GccMeanColor = (rgb *) malloc(sizeof(rgb) * 100);

  // Allocate ephemeral metrics structs
  double HSV_S_Subhistogram[360][MODE_SUBINS];
  double HSV_V_Subhistogram[360][MODE_SUBINS];

  // Initialize metrics
  for (int i=0; i < 360; i++) {
    metrics->hsv_h[i] = 0;
    metrics->SMean[i] = 0;
    metrics->VMean[i] = 0;
    for(int j=0; j<MODE_SUBINS; j++) {
      HSV_S_Subhistogram[i][j] = 0;
      HSV_V_Subhistogram[i][j] = 0;
    }
  }
  for (int i=0; i<100; i++) {
    metrics->Gcc[i] = 0;
    metrics->GccMeanColor[i] = { (double)0, (double)0, (double)0 };
  }

  // Calculate metrics (for each pixel...)
  for(int i=0; i < image->size; i = i + 3) {
    hsv HSV = get_HSV_for_pixel(i, image);
    int h = floor(HSV.h);
    metrics->hsv_h[h]++;

    double s = HSV.s;
    double v = HSV.v;
    metrics->SMean[h] += s;
    metrics->VMean[h] += v;

    int sIndex = floor(s * MODE_SUBINS);
    int vIndex = floor(v * MODE_SUBINS);
    HSV_S_Subhistogram[h][sIndex]++;
    HSV_V_Subhistogram[h][vIndex]++;

    int gccBin = get_gcc_bin_for_pixel(i, image);
    if (gccBin >= 0) {
      metrics->Gcc[gccBin]++;
      rgb RGB = get_rgb_for_pixel(i, image);
      rgb newColor = {
          metrics->GccMeanColor[gccBin].r + RGB.r,
          metrics->GccMeanColor[gccBin].g + RGB.g,
          metrics->GccMeanColor[gccBin].b + RGB.b,
      };
      metrics->GccMeanColor[gccBin] = newColor;
    }
  }

  // Finish calculation of S and V mean/mode
  for(int i=0; i < 360; i++) {
    metrics->SMean[i] /= metrics->hsv_h[i];
    metrics->VMean[i] /= metrics->hsv_h[i];

    int SMaxBin = 0, VMaxBin = 0;
    for(int j = 0; j < MODE_SUBINS; j++) {
      SMaxBin = HSV_S_Subhistogram[i][j] >= HSV_S_Subhistogram[i][SMaxBin] ? j : SMaxBin;
      VMaxBin = HSV_V_Subhistogram[i][j] >= HSV_V_Subhistogram[i][VMaxBin] ? j : VMaxBin;
    }
    metrics->SMode[i] = ((double)SMaxBin+1)/MODE_SUBINS;
    metrics->VMode[i] = ((double)VMaxBin+1)/MODE_SUBINS;
  }

  // Finish calculation of Gcc Mean
  for(int i=0; i<100; i++) {
    metrics->GccMeanColor[i] = {
      metrics->GccMeanColor[i].r / metrics->Gcc[i],
      metrics->GccMeanColor[i].g / metrics->Gcc[i],
      metrics->GccMeanColor[i].b / metrics->Gcc[i],
    };
  }

  return metrics;
}