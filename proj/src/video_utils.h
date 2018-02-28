#ifndef _LCOM_VIDEO_UTILS_H_
#define _LCOM_VIDEO_UTILS_H_

#include "utilities.h"

/** @defgroup video_utils video_utils
 * @{
 *
 * Constants and functions that are helpful for programming the video graphics controller
 */

 #define VBE_SET_MODE  0x4F02 /** VBE call to function 02 - set VBE mode*/
 #define VIDEO_INT     0x10   /** Video card interrupt*/

 //(each component goes from 0 to 31, except green that goes up to 63)
 #define MAX_RED 31
 #define MAX_GREEN 63
 #define MAX_BLUE 31

 #define FIVE_LSB_BITMASK (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
 #define SIX_LSB_BITMASK (BIT(5) | FIVE_LSB_BITMASK)
 /**
  * @brief Converts passed color components into 2 byte color (pixel)
  * @param  r The red component of the desired color
  * @param  g The green component of the desired color
  * @param  b The blue component of the desired color
  * @return   The color in 2 byte 5R 6G 5B representation
  */
 unsigned short rgb_to_pixel(int r, int g, int b);

 /**
  * @brief Converts the passed pixel in 5R 6G 5B format to the color components
  * @param pixel Pixel to interpret
  * @param r     Red component
  * @param g     Green component
  * @param b     Blue component
  */
 void pixel_to_rgb(unsigned short pixel, int * r, int * g, int * b);


/**@}*/

#endif /* _LCOM_VIDEO_UTILS_H */
