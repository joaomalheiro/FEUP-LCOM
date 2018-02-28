#pragma once

///bitmap.h and bitmap.c were taken from http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html with permission from the teacher, with some alterations
//Modifications were made to drawBitmap (and a slight one to loadBitmap, that could have caused memory leaks, and switching over to unsigned shorts - since 16 bit color mode is being used);
//drawBitmapWithoutTransparency, drawFullscreenBitmap, drawBitmapWithRotation, get_rot_x, get_rot_y, copyBitmap and collision functions were all implemented by ourselves

#include <stdbool.h>

/** @defgroup Bitmap Bitmap
 * @{
 * Functions for manipulating bitmaps, the base implementation was taken from http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html with permission, but a lot of changes were done
 */

typedef struct {
    unsigned short type; // specifies the file type
    unsigned int size; // specifies the size in bytes of the bitmap file
    unsigned int reserved; // reserved; must be 0
    unsigned int offset; // specifies the offset in bytes from the bitmapfileheader to the bitmap bits
} BitmapFileHeader;

typedef struct {
    unsigned int size; // specifies the number of bytes required by the struct
    int width; // specifies width in pixels
    int height; // specifies height in pixels
    unsigned short planes; // specifies the number of color planes, must be 1
    unsigned short bits; // specifies the number of bit per pixel
    unsigned int compression; // specifies the type of compression
    unsigned int imageSize; // size of image in bytes
    int xResolution; // number of pixels per meter in x axis
    int yResolution; // number of pixels per meter in y axis
    unsigned int nColors; // number of colors used by the bitmap
    unsigned int importantColors; // number of colors that are important
} BitmapInfoHeader;

/// Represents a Bitmap
typedef struct {
    BitmapInfoHeader bitmapInfoHeader;
    unsigned short* bitmapData;
} Bitmap;

/**
 * @brief Loads a bmp image
 *
 * @param filename Path of the image to load
 * @return Non NULL pointer to the image buffer
 */
Bitmap* loadBitmap(const char* filename);

/**
 * @brief Draws an unscaled, unrotated bitmap at the given position, in the back buffer, considering transparency
 *
 * @param bitmap bitmap to be drawn
 * @param x destiny x coord
 * @param y destiny y coord
 */
void drawBitmap(Bitmap* bitmap, int x, int y);

/**
 * @brief Draws an unscaled, unrotated bitmap at the given position, in the back buffer, not considering transparency
 * @param bmp [description]
 * @param x   [description]
 * @param y   [description]
 */
void drawBitmapWithoutTransparency(Bitmap* bmp, int x, int y);

/**
* @brief Draws a bitmap with rotation given by the passed angle
* @param oldbmp The original Bitmap to draw, from which a new, rotated one will be generated
* @param x      The x at which to draw the rotated Bitmap
* @param y      The y at which to draw the rotated Bitmap
* @param angle  The angle to draw the Bitmap with
*/
void drawBitmapWithRotation(Bitmap* oldbmp, int x, int y, double angle);

/**
 * @brief Draws a fullscreen bitmap by copying it entirely to the video buffer
 * @param bmp Fullscreen bitmap to draw
 */
void drawFullscreenBitmap(Bitmap * bmp);

/**
 * @brief Destroys the given bitmap, freeing all resources used by it.
 *
 * @param bitmap bitmap to be destroyed
 */
void deleteBitmap(Bitmap* bmp);

/**
 * @brief Returns a deep copy of the passed Bitmap
 * @param  bmp Bitmap to copy
 * @return     Copy of the passed Bitmap, or NULL if unsuccsessful
 */
Bitmap * copyBitmap(Bitmap * bmp);

/**
 * @brief Determines if two bitmaps have collided using the sprite collision method
 * @param  bmp1 The first bitmap to collide
 * @param  b1x  The x position of the first bitmap
 * @param  b1y  The y position of the first bitmap
 * @param  bmp2 The second bitmap to collide
 * @param  b2x  The x position of the second bitmap
 * @param  b2y  The y position of the second bitmap
 * @return      true if the bitmaps collided, false if they did not
 */
bool check_if_bitmaps_collided(Bitmap * bmp1, int b1x, int b1y, Bitmap * bmp2, int b2x, int b2y);

/**
 * @brief Determines if one rotated bitmap collided with a non rotated bitmap using the sprite collision method
 * @param  bmp1    The first bitmap to collide
 * @param  b1x     The x position of the first bitmap
 * @param  b1y     The y position of the first bitmap
 * @param  angleb1 The angle of the first bitmap
 * @param  bmp2    The second bitmap to collide
 * @param  b2x     The x position of the second bitmap
 * @param  b2y     The y position of the second bitmap
 * @return         true if the bitmaps collided, false if they did not
 */
bool check_if_bitmaps_collided_rotated_w_non_rotated(Bitmap * bmp1, int b1x, int b1y, double angleb1, Bitmap * bmp2, int b2x, int b2y);

/**@}*/
