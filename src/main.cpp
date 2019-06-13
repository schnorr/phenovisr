#include <Rcpp.h>
#include <jpeglib.h>
#include <sys/time.h>
#include <math.h>
#include <string>
#include "jpeg_image.h"
#include "metrics_extraction.h"

using namespace Rcpp;

static image_t *global_mask = NULL;

// [[Rcpp::export]]
void phenovis_read_mask(std::string maskname)
{
  if (global_mask){
    free(global_mask->image);
    free(global_mask);
    global_mask = NULL;
  }
  global_mask = load_jpeg_image(maskname.c_str());
}

// [[Rcpp::export]]
DataFrame phenovis_get_mean_gcc(StringVector images) {
  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("Unmasked_Pixels");
  columnNames.push_back("Mean_Gcc");

  NumericMatrix matrix(images.size(), 4);

  // names is a vector to keep image names
  std::vector<std::string> names;

  int i, row_number = 0;
  for (i = 0; i < images.size(); i++) {
    // Load the image and apply mask
    image_t *image = load_jpeg_image(std::string(images(i)).c_str());
    int considered_pixels = image->width * image->height;
    if (global_mask) {
      considered_pixels = apply_mask(image, global_mask);
    }

    // Calculate the mean GCC
    double mean_gcc = get_mean_gcc_for_image(image);

    // Push back the image name (to aligh to this row)
    names.push_back(std::string(images(i)));
    NumericVector row;
    row.push_back(image->width);
    row.push_back(image->height);
    row.push_back(considered_pixels);
    row.push_back(mean_gcc);

    matrix.row(row_number) = row;
    row_number++;

    //Free the image data
    free(image->image);
    free(image);
  }

  // Create the resulting data frame
  DataFrame ret(matrix);
  ret.insert(ret.begin(), names);
  columnNames.push_front("Picture.Path");
  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
}

// [[Rcpp::export]]
DataFrame phenovis_get_metrics(StringVector images) {
  CharacterVector columnNames;
  columnNames.push_back("Considered_Pixels");
  columnNames.push_back("HSV_Bin");
  columnNames.push_back("HSV_H");
  columnNames.push_back("HSV_SMean");
  columnNames.push_back("HSV_VMean");
  columnNames.push_back("HSV_SMode");
  columnNames.push_back("HSV_VMode");
  columnNames.push_back("Gcc_Bin");
  columnNames.push_back("Gcc_Value");
  columnNames.push_back("Gcc_Mean_R");
  columnNames.push_back("Gcc_Mean_G");
  columnNames.push_back("Gcc_Mean_B");

  int rows = (images.size() * (360 + 100));
  NumericMatrix matrix(rows, 12);

  // names is a string vector to keep image names
  std::vector<std::string> names;
  std::vector<std::string> metricNames;

  int i, row_number = 0;
  for (i = 0; i < images.size(); i++) {
    // Load the image and apply mask
    image_t *image = load_jpeg_image(std::string(images(i)).c_str());

    std::vector<int> unmaskedPixels = (!!global_mask) 
      ? get_unmasked_pixels(global_mask) 
      : get_all_pixels(image);

    // Calculate the HSV_H metric
    phenology_metrics_t *phenology_metrics;
    phenology_metrics = calculate_image_metrics(image, unmaskedPixels);

    // Push HSV_H metrics into the matrix
    for (int j=0; j < 360; j++) {
      // Push back the image and metric names
      names.push_back(std::string(images(i)));
      metricNames.push_back("HSV");

      NumericVector row;
      row.push_back(phenology_metrics->consideredPixels);
      row.push_back(j);
      row.push_back(phenology_metrics->hsv_h[j]);
      row.push_back(phenology_metrics->SMean[j]);
      row.push_back(phenology_metrics->VMean[j]);
      row.push_back(phenology_metrics->SMode[j]);
      row.push_back(phenology_metrics->VMode[j]);

      // Empty values to fill in Gcc gap
      // TODO Test if R will assume NA if I ommit these lines. If not, how can I push NA values?
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);

      matrix.row(row_number) = row;
      row_number++;
    }

    // Push Gcc metrics into the matrix
    for (int j=0; j<100; j++) {
      // Push back the image and metric names
      names.push_back(std::string(images(i)));
      metricNames.push_back("Gcc");

      NumericVector row;
      // Empty stuff to fill in the HSV values
      row.push_back(phenology_metrics->consideredPixels);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);
      row.push_back(-1);

      row.push_back(j);
      row.push_back(phenology_metrics->Gcc[j]);
      row.push_back(phenology_metrics->GccMeanColor[j].r);
      row.push_back(phenology_metrics->GccMeanColor[j].g);
      row.push_back(phenology_metrics->GccMeanColor[j].b);

      matrix.row(row_number) = row;
      row_number++;
    }

    // Free the calculated metrics
    free(phenology_metrics->hsv_h);
    free(phenology_metrics->SMean);
    free(phenology_metrics->VMean);
    free(phenology_metrics->SMode);
    free(phenology_metrics->VMode);
    free(phenology_metrics->Gcc);
    free(phenology_metrics->GccMeanColor);
    free(phenology_metrics);

    //Free the image data
    free(image->image);
    free(image);
  }

  // Create the resulting data frame
  DataFrame ret(matrix);
  
  //Add names and metric names to the beginning
  ret.insert(ret.begin(), metricNames);
  columnNames.push_front("Metric_Type");
  ret.insert(ret.begin(), names);
  columnNames.push_front("Picture.Path");

  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
}