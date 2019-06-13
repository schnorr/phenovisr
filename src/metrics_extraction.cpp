#include "metrics_extraction.h"
#include "rgb2hsv.h"

#define MODE_SUBINS 10

double get_mean_gcc_for_image(image_t *image)
{
  double gcc_sum = 0;
  int consideredPixels = 0;
  // For every pixel...
  for (int i = 0; i < image->size; i += 3) {
    rgb RGB = get_rgb_for_pixel(i, image);
    if (!is_black(RGB)) {
      gcc_sum += get_gcc_value(RGB);
      consideredPixels++;
    }
  }
  return gcc_sum / consideredPixels;
}

phenology_metrics_t *calculate_image_metrics(image_t *image, std::vector<int> unmaskedPixels) {
  // Allocate metrics struct
  phenology_metrics_t *metrics = (phenology_metrics_t*) malloc(sizeof(phenology_metrics_t));
  metrics->consideredPixels = 0;
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

  // Calculate metrics (for each unmasked pixel...)
  for(int i=0; i < unmaskedPixels.size(); i++) {
    int pixelIndex = unmaskedPixels[i];
    rgb RGB = get_rgb_for_pixel(pixelIndex, image);

    if(!is_black(RGB)) {
      metrics->consideredPixels++;

      // HSV Computations
      hsv HSV = get_HSV_for_pixel(pixelIndex, image);
      int h = floor(HSV.h);
      metrics->hsv_h[h]++;
      metrics->SMean[h] += HSV.s;
      metrics->VMean[h] += HSV.v;

      int sIndex = floor(HSV.s * MODE_SUBINS);
      int vIndex = floor(HSV.v * MODE_SUBINS);
      HSV_S_Subhistogram[h][sIndex]++;
      HSV_V_Subhistogram[h][vIndex]++;

      // Gcc Computations
      int gccBin = get_gcc_bin_for_pixel(pixelIndex, image);
      metrics->Gcc[gccBin]++;
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