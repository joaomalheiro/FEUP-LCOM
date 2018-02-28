#ifndef __SPRITE_H
#define __SPRITE_H

#include "bitmap.h"

/** @defgroup sprite sprite
 * @{
 *
 * Sprite Module, making animations from Bitmaps
 */

typedef struct {
  int frames_per_bitmap;          //Frames between bitmaps
  int frames_left;                //Frames left to change bitmap
  int n_bitmaps;                  //Total number of bitmaps
  int current_bitmap;             //Current bitmap
  Bitmap ** bmps;                 //Array of all the bitmaps
} Sprite;

/**
 * @brief Sprite Object Constructor
 * @param  bmp_paths         The paths to the bmps to load
 * @param  n_bmps            The number of bmps to load
 * @param  frames_per_bitmap The number of frames to draw each bitmap
 * @return                   Returns a pointer to a valid Sprite or NULL in case of failure
 */
Sprite * create_sprite(char** bmp_paths, int n_bmps, int frames_per_bitmap);

/**
 * @brief Sprite Object Destructor
 * @param s_ptr Sprite Object to destroy
 */
void destroy_sprite (Sprite ** s_ptr);

/**
 * @brief Draws a Sprite and updates internal state (moving to next frame, etc)
 * @param s_ptr Sprite to draw
 * @param x     X at which to draw the sprite
 * @param y     Y at which to draw the sprite
 */
void draw_sprite(Sprite * s_ptr, int x, int y);

/**
 * @brief Draws a Sprite with rotation and updates internal state (moving to next frame, etc)
 * @param s_ptr Sprite to draw with rotation
 * @param x     X at which to draw the sprite
 * @param y     Y at which to draw the sprite
 * @param angle Angle, in radians, at which to draw the sprite
 */
void draw_sprite_wRotation(Sprite * s_ptr, int x, int y, double angle);

/**
 * @brief Gets the current Bitmap of the Sprite, useful for collisions
 * @param  s_ptr Sprite to get the current bitmap for
 * @return       Returns the current Bitmap of the passed sprite
 */
Bitmap * sprite_get_current_bitmap(Sprite * s_ptr);


#endif /* __SPRITE_H */
