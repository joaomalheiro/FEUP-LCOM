#ifndef _GUARD_H
#define _GUARD_H

#include <stdbool.h>
#include "bitmap.h"
#include "checkpoint.h"

/** @defgroup guard guard
 * @{
 *
 * Functions and structs that represent and operate over a Guard
 */

typedef enum {
  UP = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3,
  DEAD
} guard_direction_enum;

typedef struct {
  long guardX;
  long guardY;
  int speedX;
  int speedY;
  int current_checkpoint;
  int n_checkpoints;
  bool isCyclical;
  bool goingForward;
  Checkpoint ** checkpoints;
  guard_direction_enum currdirection;
  Bitmap * guardSprites[4];
} Guard;

/**
 * @brief Guard Object Constructor
 * @param  checkpoints  The Checkpoints that the guard should go through
 * @param  ncheckpoints The number of checkpoints in the checkpoints array
 * @param  isCyclical   If the guard will be cyclical or "back and forth"
 * @return              Returns a pointer to a valid Guard Object or NULL in case of failure
 */
Guard * create_guard(Checkpoint checkpoints[], int ncheckpoints, bool isCyclical);

/**
 * @brief Draws a guard
 * @param g_ptr Guard Object to draw
 */
void draw_guard(Guard * g_ptr);

/**
 * @brief Guard Object Destructor
 * @param g_ptr Guard Object to destroy
 */
void destroy_guard(Guard ** g_ptr);

/**
 * @brief Updates a guard by moving it in between checkpoints
 * @param g_ptr Guard Object to update
 */
void update_guard(Guard * g_ptr);

/**
 * @brief Gets the guard's current bitmap, useful for pixel-perfect collision detection
 * @param  g_ptr Guard whose bitmap to get
 * @return       Current Bitmap of the Guard passed
 */
Bitmap * get_guard_current_bitmap(Guard * g_ptr);

#endif /* __GUARD_H */
