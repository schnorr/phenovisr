#include <Rcpp.h>
#include <jpeglib.h>
#include <sys/time.h>
#include <math.h>
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

// [[Rcpp::export]]
DataFrame phenovis_get_gcc_histogram(StringVector names, int number_of_bins)
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
    int *histogram = get_gcc_histogram (number_of_bins, Green, image);
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
DataFrame phenovis_get_histogram(StringVector names, int number_of_bins)
{
  return phenovis_get_gcc_histogram (names, number_of_bins);
}
