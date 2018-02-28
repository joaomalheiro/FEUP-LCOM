#include "video_utils.h"

unsigned short rgb_to_pixel(int r, int g, int b) {
  if(r < 0){
    r = 0;
  } else if(r > MAX_RED){
    r = MAX_RED;
  }

  if(g < 0){
    g = 0;
  } else if(g > MAX_GREEN){
    g = MAX_GREEN;
  }

  if(b < 0){
    b = 0;
  } else if(b > MAX_BLUE){
    b = MAX_BLUE;
  }

  return ((r << 11) | (g << 5) | b);
}

void pixel_to_rgb(unsigned short pixel, int * r, int * g, int * b) {  
  *r = (pixel >> 11) & FIVE_LSB_BITMASK;
  *g = (pixel >> 5) & SIX_LSB_BITMASK;
  *b = (pixel) & FIVE_LSB_BITMASK;
}
