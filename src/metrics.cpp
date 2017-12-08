#include "metrics.h"

static double get_rcc (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (double)r/(double)(r+g+b);
}

static double get_gcc (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (double)g/(double)(r+g+b);
}

static double get_bcc (unsigned char r, unsigned char g, unsigned char b)
{
  return (r+g+b) == 0 ? 0 : (double)b/(double)(r+g+b);
}

static double get_H (unsigned char r, unsigned char g, unsigned char b)
{
  double R = (double)r/255;
  double G = (double)g/255;
  double B = (double)b/255;
  rgb RGB = {R, G, B};
  hsv HSV = rgb2hsv(RGB);
  return HSV.h/360;
}

double get_metric (PGAMetricType type, unsigned char r, unsigned char g, unsigned char b)
{
  double ret;
  switch (type){
  case Red: ret = get_rcc (r, g, b); break;
  case Green: ret = get_gcc (r, g, b); break;
  case Blue: ret = get_bcc (r, g, b); break;
  case H: ret = get_H (r, g, b); break;
  case Undef:
  default: ret = get_gcc (r, g, b); break;
  }
  }
  return ret;
}

int is_black (unsigned char r, unsigned char g, unsigned char b)
{
  return !(r+g+b);
}
