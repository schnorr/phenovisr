#include <Rcpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <sys/time.h>
#include <math.h>

using namespace Rcpp;

typedef enum { Red, Green, Blue, Undef } PGAMetricType;
typedef struct image {
  unsigned char *image;
  int width;
  int height;
  unsigned long size;
  int channels;
}image_t;

image_t *load_jpeg_image (const char *filename)
{
  image_t *image;        // data for the image
  
  struct jpeg_decompress_struct info; //for our jpeg info
  struct jpeg_error_mgr err;          //the error handler
  FILE *file = fopen(filename, "rb");  //open the file
  if(!file) {
     return NULL;
  }
  info.err = jpeg_std_error(&err);
  jpeg_create_decompress(&info);   //fills info structure

  jpeg_stdio_src(&info, file);    
  jpeg_read_header(&info, TRUE);   // read jpeg file header
  jpeg_start_decompress(&info);    // decompress the file

  image = (image_t*)malloc(sizeof(image_t));
  image->width = info.output_width;
  image->height = info.output_height;
  image->channels = info.num_components;

  // std::cout << "Filename: " << filename << std::endl;
  // std::cout << "Width: " << image->width << std::endl;
  // std::cout << "Height: " << image->height << std::endl;
  // std::cout << "Channels: " << image->channels << std::endl;

  image->size = image->width * image->height * image->channels;
  image->image = (unsigned char *)malloc(image->size);
  
  while (info.output_scanline < info.output_height){
    unsigned char *rowptr = &image->image[info.output_scanline * image->width * image->channels];
    jpeg_read_scanlines(&info, &rowptr, 1);
  }
  jpeg_finish_decompress(&info);   //finish decompressing
  jpeg_destroy_decompress(&info);
  fclose(file);
  return image; /* should be freed */
}

int apply_mask (image_t *image, image_t *mask)
{
  if (image->width != mask->width || image->height != image->height) {
    printf ("Image and Mask have different dimensions. Error.\n");
    return -1;
  }

  if (image->channels != mask->channels){
    printf ("Image and Mask have different number of channels. Error.\n");
    return -1;
  }

  int total_after_mask = 0;
  for (int i = 0; i < mask->size; i = i+3) {
    unsigned char r = mask->image[i+0];
    unsigned char g = mask->image[i+1];
    unsigned char b = mask->image[i+2];
    if (!(r == 255 && g == 255 && b == 255)){
      image->image[i+0] = 0;
      image->image[i+1] = 0;
      image->image[i+2] = 0;
    }else{
      total_after_mask++;
    }
  }
  return total_after_mask;
}


static float get_red_average (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (float)r/(float)(r+g+b);
}

static float get_green_average (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (float)g/(float)(r+g+b);
}

static float get_blue_average (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (float)b/(float)(r+g+b);
}

float get_average (PGAMetricType type, unsigned char r, unsigned char g, unsigned char b)
{
  switch (type){
  case Red: return get_red_average (r, g, b);
  case Green: return get_green_average (r, g, b);
  case Blue: return get_blue_average (r, g, b);
  case Undef:
  default: return get_green_average (r, g, b);
  }
  return 0;
}

int is_black (unsigned char r, unsigned char g, unsigned char b)
{
  return !(r+g+b);
}

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
DataFrame phenovis_get_histogram(StringVector names, int number_of_bins)
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
    int *histogram = get_metric (number_of_bins, Green, image);
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
