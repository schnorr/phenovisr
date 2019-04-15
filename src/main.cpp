#include <Rcpp.h>
#include <jpeglib.h>
#include <sys/time.h>
#include <math.h>
#include <string>
#include "jpeg_image.h"
#include "metrics.h"
#include "gcc.h"
#include "rgb2hsv.h"
#include "metrics_extraction.h"

using namespace Rcpp;

static image_t *global_mask = NULL;

// [[Rcpp::export]]
int phenovis_red(){
  return Red;
}

// [[Rcpp::export]]
int phenovis_green(){
  return Green;
}

// [[Rcpp::export]]
int phenovis_blue(){
  return Blue;
}

// [[Rcpp::export]]
int phenovis_H(){
  return H;
}

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

static DataFrame _phenovis_get_histogram(PGAMetricType type, StringVector names, int number_of_bins)
{
  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("Pixels");
  int j;
  for (j = 0; j < number_of_bins; j++){
    columnNames.push_back("B" + std::to_string(j));
  }  
  
  IntegerMatrix mat(names.size(), 3+number_of_bins);
  
  int i;
  for (i = 0; i < names.size(); i++){

    image_t *image = load_jpeg_image(std::string(names(i)).c_str());
    
    int considered_pixels = image->width * image->height;
    if (global_mask){
      considered_pixels = apply_mask(image, global_mask);
    }

    IntegerVector row;
    
    row.push_back(image->width);
    row.push_back(image->height);
    row.push_back(considered_pixels);
    int *histogram = get_histogram (type, number_of_bins, image);
    int j;
    for (j = 0; j < number_of_bins; j++){
      row.push_back(histogram[j]);
    }
    free(histogram);
    mat.row(i) = row;

    free(image->image);
    free(image);
  }
  DataFrame ret(mat);
  ret.insert(ret.begin(), names);
  columnNames.push_front("Name");
  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
}

// [[Rcpp::export]]
DataFrame phenovis_get_histogram(int mtype, StringVector names, int number_of_bins)
{
  PGAMetricType type = static_cast<PGAMetricType>(mtype);
  if (type < Undef){
    return _phenovis_get_histogram (type, names, number_of_bins);
  }else{
    return DataFrame();
  }
}

// [[Rcpp::export]]
DataFrame phenovis_get_gcc_color_histogram(StringVector names, int numberOfBins) {
  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("Pixels");
  columnNames.push_back("GCC_bin");
  columnNames.push_back("Bin_value");
  columnNames.push_back("Bin_color_r");
  columnNames.push_back("Bin_color_g");
  columnNames.push_back("Bin_color_b");

  NumericMatrix mat(names.size() * numberOfBins, 8);

  int i;
  for (i = 0; i < names.size(); i++) {
    // For every image...
    image_t *image = load_jpeg_image(std::string(names(i)).c_str());
    int considered_pixels = image->width * image->height;
    if (global_mask) {
      considered_pixels = apply_mask(image, global_mask);
    }

    gcc_histogram_t *histogram = get_gcc_color_histogram(numberOfBins, image);
    for(int j = 0; j < numberOfBins; j++) {
      NumericVector row;
      // For every histogram bin
      row.push_back(image->width);
      row.push_back(image->height);
      row.push_back(considered_pixels);
      row.push_back(j);
      row.push_back(histogram->gcc[j]);
      row.push_back(histogram->color_histogram[j].r);
      row.push_back(histogram->color_histogram[j].g);
      row.push_back(histogram->color_histogram[j].b);
      
      // Add row to the final data frame
      mat.row((i * numberOfBins) + j) = row;
    }
    
    free(histogram->gcc);
    free(histogram->color_histogram);
    free(histogram);

    free(image->image);
    free(image);
  }
  DataFrame ret(mat);
  StringVector repeatedNames;
  for(int i=0; i<names.size(); i++) {
    for(int j=0; j<numberOfBins; j++) {
      repeatedNames.push_back(names(i));
    }
  }
  ret.insert(ret.begin(), repeatedNames);
  columnNames.push_front("Name");
  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
}

// [[Rcpp::export]]
DataFrame phenovis_get_HSV_mean_histogram (StringVector images) {
  // The bins represent each H value, so the number is is fixed in 360.
  int nbins = 360;

  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("UnmaskedPixels");
  columnNames.push_back("H");
  columnNames.push_back("HCount");
  columnNames.push_back("SMean");
  columnNames.push_back("VMean");
  
  NumericMatrix matrix(images.size() * nbins, 7);

  // names is a vector to keep the image names
  std::vector<std::string> names;

  int i;
  int row_number = 0;
  for (i=0; i < images.size(); i++) {
    // Load image and apply mask
    image_t *image = load_jpeg_image(std::string(images(i)).c_str());
    int consideredPixels = image->width * image->height;
    if(global_mask) {
      consideredPixels = apply_mask(image, global_mask);
    }

    // Calculate the histogram
    HSV_Mean_Histogram_t *meanHistogram;
    meanHistogram = get_hsv_mean_histogram(image, consideredPixels);

    // Put histogram data in the created matrix
    for (int j = 0; j < nbins; j++) {
      // Push back the image name
      names.push_back(std::string(images(i)));

      NumericVector row;
      row.push_back(image->width);
      row.push_back(image->height);
      row.push_back(consideredPixels);
      row.push_back(meanHistogram[j].H);
      row.push_back(meanHistogram[j].HCount);
      row.push_back(meanHistogram[j].SMean);
      row.push_back(meanHistogram[j].VMean);
      matrix.row(row_number) = row;
      row_number++;
    }

    // Memory cleanup
    free(meanHistogram);
    free(image->image);
    free(image);
  }

  // Create the resulting data frame
  DataFrame dataFrame(matrix);
  dataFrame.insert(dataFrame.begin(), names);
  columnNames.push_front("Name");
  dataFrame.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(dataFrame);
}

// [[Rcpp::export]]
DataFrame phenovis_get_HSV_mode_histogram(StringVector images)
{
  // The bins represent each H value, so the number is is fixed in 360.
  int nbins = 360;

  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("UnmaskedPixels");
  columnNames.push_back("H");
  columnNames.push_back("HCount");
  columnNames.push_back("SMode");
  columnNames.push_back("VMode");

  NumericMatrix matrix(images.size() * nbins, 7);

  // names is a vector to keep the image names
  std::vector<std::string> names;

  int i;
  int row_number = 0;
  for (i = 0; i < images.size(); i++)
  {
    // Load image and apply mask
    image_t *image = load_jpeg_image(std::string(images(i)).c_str());
    int consideredPixels = image->width * image->height;
    if (global_mask) {
      consideredPixels = apply_mask(image, global_mask);
    }

    // Calculate the histogram
    HSV_Mode_Histogram_t *modeHistogram;
    modeHistogram = get_hsv_mode_histogram(image, consideredPixels);

    // Put histogram data in the created matrix
    for (int j = 0; j < nbins; j++) {
      // Push back the image name
      names.push_back(std::string(images(i)));

      NumericVector row;
      row.push_back(image->width);
      row.push_back(image->height);
      row.push_back(consideredPixels);
      row.push_back(modeHistogram[j].H);
      row.push_back(modeHistogram[j].HCount);
      row.push_back(modeHistogram[j].SMode);
      row.push_back(modeHistogram[j].VMode);
      matrix.row(row_number) = row;
      row_number++;
    }

    // Memory cleanup
    free(modeHistogram);
    free(image->image);
    free(image);
  }

  // Create the resulting data frame
  DataFrame dataFrame(matrix);
  dataFrame.insert(dataFrame.begin(), names);
  columnNames.push_front("Name");
  dataFrame.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(dataFrame);
}

// [[Rcpp::export]]
DataFrame phenovis_get_HSV_double_histogram (int mtype, StringVector images, int nsubins)
{
  //The number of bins for H is fixed
  int nbins = 360;

  PGAMetricType type = static_cast<PGAMetricType>(mtype);
  if(type != H){
    std::cout << "A type different of phenovis_H() is unsupported for " <<  __FUNCTION__ << std::endl;
    return DataFrame();
  }

  CharacterVector columnNames;
  columnNames.push_back("Width");
  columnNames.push_back("Height");
  columnNames.push_back("Pixels");
  columnNames.push_back("H");
  columnNames.push_back("Count");
  for (int j = 0; j < nsubins; j++){
    columnNames.push_back("V" + std::to_string(j));
  }

  IntegerMatrix mat(images.size() * nbins, 5 + nsubins);
  // names is a vector to keep image names
  std::vector<std::string> names;

  int i;
  int row_number = 0;
  for (i = 0; i < images.size(); i++){

    // Load image and apply mask
    image_t *image = load_jpeg_image(std::string(images(i)).c_str());
    int considered_pixels = image->width * image->height;
    if (global_mask){
      considered_pixels = apply_mask(image, global_mask);
    }

    // Calculate the histogram
    hsv_histogram_t *HIST;
    HIST = get_HSV_double_histogram (type, image, nbins, nsubins);

    for (int j = 0; j < nbins; j++){
      // Push back the image name (to align to this row)
      names.push_back(std::string(images(i)));

      IntegerVector row;
      row.push_back(image->width);
      row.push_back(image->height);
      row.push_back(considered_pixels);
      row.push_back(HIST[j].H);
      row.push_back(HIST[j].count);
      for (int k = 0; k < HIST[j].nsubins; k++){
        if (HIST[j].V == NULL) {
          row.push_back(0);
	      } else {
	        row.push_back(HIST[j].V[k]);
	      }
      }
      mat.row(row_number) = row;
      row_number++;
    }

    // Free the double HIST
    for (int j = 0; j < nbins; j++){
      free(HIST[j].V);
    }
    free(HIST);

    // Free the image data
    free(image->image);
    free(image);
  }

  // Create the resulting data frame
  DataFrame ret(mat);
  ret.insert(ret.begin(), names);
  columnNames.push_front("Name");
  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
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
  columnNames.push_front("Name");
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
    int considered_pixels = image->width * image->height;
    if (global_mask) {
      considered_pixels = apply_mask(image, global_mask);
    }

    // Calculate the HSV_H metric
    phenology_metrics_t *phenology_metrics;
    phenology_metrics = calculate_image_metrics(image);

    // Push HSV_H metrics into the matrix
    for (int j=0; j < 360; j++) {
      // Push back the image and metric names
      names.push_back(std::string(images(i)));
      metricNames.push_back("HSV");

      NumericVector row;
      row.push_back(considered_pixels);
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
      row.push_back(considered_pixels);
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
  columnNames.push_front("Name");

  ret.attr("names") = columnNames;
  Function asDF("as.data.frame");
  return asDF(ret);
}