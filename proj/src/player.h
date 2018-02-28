#ifndef _PLAYER_H
#define _PLAYER_H

#include "bitmap.h"
#include "sprite.h"

/** @defgroup player player
 * @{
 *
 * Player module, with functions and structs that represent and operate over a Player
 */

////Useful player constants
//Default movement speed for the player (to use in event handling, when changing player speed)
#define PLAYER_MOVE_SPEED 5
#define PLAYER_ROTATION_SPEED 0.1
#define PLAYER_START_ROTATION 0

typedef struct {
  bool isMoving;
  double angle;
  long x;
  long y;
  int speedX;
  int speedY;
  Sprite * playerSprite;
  //Bitmap * playerHitbox
} Player;

/**
 * @brief Player Object Constructor
 * @param  startx The x at which to start
 * @param  starty The y at which to start
 * @param  speedx The speed in the x direction at which to start
 * @param  speedy The speed in the y direction at which to start
 * @param  angle  The angle at which to start
 * @return        Returns a pointer to a valid Player Object or NULL in case of failure
 */
Player * create_player(long startx, long starty, int speedx, int speedy, double angle);

///Updates player internal speed
/**
 * @brief Sets the current player speed to the passed values
 * @param p_ptr  Player to alter
 * @param speedX New x speed
 * @param speedY New y speed
 */
void set_player_speed(Player * p_ptr, int speedX, int speedY);
/**
 * @brief Sets the current player x speed to the passed value
 * @param p_ptr  Player to alter
 * @param speedX New x speed
 */
void set_player_speedX(Player * p_ptr, int speedX);
/**
 * @brief Sets the current player y speed to the passed value
 * @param p_ptr  Player to alter
 * @param speedY New y speed
 */
void set_player_speedY(Player * p_ptr, int speedY);

//Updates player speed based on direction (which is given by the function called)
/**
 * @brief Sets the Player to be moving to the right
 * @param p_ptr Player to alter
 */
void set_player_moving_right(Player * p_ptr);
/**
 * @brief Sets the Player to be moving to the left
 * @param p_ptr Player to alter
 */
void set_player_moving_left(Player * p_ptr);
/**
 * @brief Sets the Player to be moving up
 * @param p_ptr Player to alter
 */
void set_player_moving_up(Player * p_ptr);
/**
 * @brief Sets the Player to be moving down
 * @param p_ptr Player to alter
 */
void set_player_moving_down(Player * p_ptr);
/**
 * @brief Sets the player to be stopped in the X direction
 * @param p_ptr Player to alter
 */
void set_player_stopped_x(Player * p_ptr);
/**
 * @brief Sets the player to be stopped in the Y direction
 * @param p_ptr Player to alter
 */
void set_player_stopped_y(Player * p_ptr);

//Updates player rotation
/**
 * @brief Sets the Player angle to the passed in value (To be used with mouse movement)
 * @param p_ptr Player to alter
 * @param angle New player angle
 */
void set_player_angle(Player * p_ptr, double angle);

/**
 * @brief Updates Player position based on internal speed
 * @param p_ptr Player to update
 */
void update_player(Player * p_ptr);

/**
 * @brief Draws a player based on its coordinates and angle and updates its animation frames
 * @param p_ptr Player to draw
 */
void draw_player(Player * p_ptr);

//
/**
 * @brief Gets the current player bitmap, useful for collision detection
 * @param  p_ptr Player to get the current bitmap for
 * @return       Returns the current player bitmap
 */
Bitmap * get_player_current_bitmap(Player * p_ptr);

/**
 * @brief Player Object Destructor. A Player ** is passed to set the pointer itself to NULL, to enable knowing if the "object" is trully deallocated
 * @param p_ptr Player to destroy
 */
void destroy_player(Player ** p_ptr);

#endif /* __PLAYER_H */
