#include <Rcpp.h>
#include <jpeglib.h>
#include <sys/time.h>
#include <math.h>
#include <string>
#include "jpeg_image.h"
#include "metrics.h"
#include "gcc.h"

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
