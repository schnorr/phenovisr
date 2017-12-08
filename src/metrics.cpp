#include "metrics.h"

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

static float get_H (unsigned char r, unsigned char g, unsigned char b)
{
  double R = (int)r/255;
  double G = (int)g/255;
  double B = (int)b/255;

  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);
  return HSV.h;
}

float get_metric (PGAMetricType type, unsigned char r, unsigned char g, unsigned char b)
{
  switch (type){
  case Red: return get_red_average (r, g, b);
  case Green: return get_green_average (r, g, b);
  case Blue: return get_blue_average (r, g, b);
  case H: return get_H (r, g, b);
  case Undef:
  default: return get_green_average (r, g, b);
  }
  return 0;
}

int is_black (unsigned char r, unsigned char g, unsigned char b)
{
  return !(r+g+b);
}
