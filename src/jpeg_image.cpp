#include "jpeg_image.h"

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
