#ifndef __LEVEL_H
#define __LEVEL_H

#include <stdbool.h>
#include "bitmap.h"
#include "player.h"
#include "guard.h"
#include "sprite.h"
#include "robinix.h"

struct Robinix;

/** @defgroup level level
 * @{
 *
 * Functions and structs pertaining over main gameplay, used while the user is "in game"
 */

typedef struct {
  long x;
  long y;
  //To change to sprite if an animation is desired
  Bitmap * bmp;
  bool picked_up;
} Treasure;

typedef struct {
  long x;
  long y;
  Bitmap * bmp;
  bool picked_up;
} Coin;

typedef enum {
  EXIT_SUPERLOCKED,
  EXIT_CLOSED,
  EXIT_OPEN
} exit_state_enum;

typedef struct {
  long x;
  long y;
  Bitmap * superlocked_sprite;
  Bitmap * closed_sprite;
  Sprite * open_sprite;
  exit_state_enum exit_state;
} Exit;

typedef struct {
  long x;
  long y;
  Bitmap * closed_bmp;
  Bitmap * open_bmp;
  bool closed;
  bool hovered;
} Door;

typedef enum {
  M_OVER_NOTHING = 0,
  M_OVER_DOOR,
  M_OVER_ENUM_SIZE
} mouse_over_enum;

typedef struct Level {
  //Pointer to a player (Pointer to allow allocation and deallocation)
  Player * player;
  //Indicates the number of guards (size of array below)
  unsigned int n_guards;
  //Array of pointers to guards (Pointers so they can be deallocated)
  Guard ** guards;
  //Pointer to a treasure
  Treasure * treasure;
  //Indicates the number of coins
  unsigned int n_coins;
  //Array of pointers to coins
  Coin ** coins;
  //Indicates the number of doors
  unsigned int n_doors;
  //Array of of pointers to doors
  Door ** doors;
  //Pointer to an exit
  Exit * exit;
  //Bitmap of the background during gameplay
  Bitmap * background_bmp;
  //The level walls bitmap
  Bitmap * level_walls;
  //A level border to ensure that the player does not laeve the screen
  Bitmap * level_border;
  //What the mouse is currently over
  mouse_over_enum current_mouse_over;
  //Bitmaps of the mouse cursor, drawn depending on the mouse being over different things
  Bitmap * mouse_bmps[M_OVER_ENUM_SIZE];
} Level;

/**
 * @brief Level Object Constructor. Constructs a Level Object based on the arguments
 * @param  level_n The level ID to construct
 * @param  is_mp   If the level is multiplayer
 * @return         Returns a pointer to a valid Level Object or NULL in case of failure
 */
Level * create_level(int level_n, bool is_mp);

/**
 * @brief Level Object Destructor
 * @param l_ptr Level Object to destroy
 */
void destroy_level(Level ** l_ptr);

/**
 * @brief Draws a Level Object (Mouse coordinates are necessary since the Level Object handles mouse drawing while playing)
 * @param l_ptr  Level object to draw
 * @param mouseX Current Mouse X
 * @param mouseY Current Mouse Y
 */
void draw_level(Level * l_ptr, long mouseX, long mouseY);

/**
 * @brief Updates a Level
 * @param l_ptr Level to Update
 * @param rob   "Father" Robinix Game Object - Necessary due to sending events on collisions
 */
void update_level(Level * l_ptr, struct Robinix * rob);

/**
 * @brief Updates mouse overs for a Level
 * @param l_ptr  Level to update mouse overs for
 * @param mouseX Current Mouse X
 * @param mouseY Current Mouse Y
 */
void level_update_mouse_over(Level * l_ptr, long mouseX, long mouseY);

/**
 * @brief Handles mouse clicks when inside a Level (Robinix object is not necessary since no Events ocurr)
 * @param l_ptr Level to handle clicks in
 */
void level_handle_mouse_click(Level * l_ptr);

/**
 * @brief For when in multiplayer mode the other player grabs the treasure, we must update the exit state
 * @param l_ptr Level to update
 */
void level_remote_got_treasure(Level * l_ptr);

//Other objects internal to Level (structs defined above)

/**
 * @brief Constructor for Treasure Object
 * @param  startx x where the treasure will be created
 * @param  starty y where the treasure will be created
 * @return        Returns a pointer to a valid Treasure object or NULL in case of failure
 */
Treasure * create_treasure(long startx, long starty);
/**
 * @brief Draws a Treasure object
 * @param t_ptr Treasure to draw
 */
void draw_treasure(Treasure * t_ptr);
/**
 * @brief Destructor for the Treasure Object
 * @param t_ptr Treasure to destroy
 */
void destroy_treasure(Treasure ** t_ptr);

/**
 * @brief Constructor for the Coin Object
 * @param  startx x where the coin will be created
 * @param  starty y where the coin will be created
 * @return        Returns a pointer to a valid Coin object or NULL in case of failure
 */
Coin * create_coin(long startx, long starty);
/**
 * @brief Draws a Coin Object
 * @param c_ptr Coin to draw
 */
void draw_coin(Coin * c_ptr);
/**
 * @brief Destructor for the Coin Object
 * @param c_ptr Coin to destroy
 */
void destroy_coin(Coin ** c_ptr);

/**
 * @brief Constructor for the Exit Object
 * @param  startx      x where the Exit will be created
 * @param  starty      y where the Exit will be created
 * @param  superlocked if the exit should start superlocked (for use in multiplayer)
 * @return             Returns a pointer to a valid Exit object or NULL in case of failure
 */
Exit * create_exit(long startx, long starty, bool superlocked);
/**
 * @brief Draws an Exit Object
 * @param ex_ptr Exit to draw
 */
void draw_exit(Exit * ex_ptr);
/**
 * @brief Moves a Exit Object into its next state
 * @param ex_ptr Exit to alter
 */
void exit_goto_next_state(Exit * ex_ptr);
/**
 * @brief Destructor for the Exit Object
 * @param ex_ptr Exit to destroy
 */
void destroy_exit(Exit ** ex_ptr);

/**
 * @brief Constructor for the Door Object
 * @param  startx          x where the Door will be created
 * @param  starty          y where the Door will be created
 * @param  closed_at_start If the door should start closed
 * @return                 Returns a pointer to a valid Door object or NULL in case of failure
 */
Door * create_door(long startx, long starty, bool closed_at_start);
/**
 * @brief Draws a Door Object
 * @param d_ptr Door to draw
 */
void draw_door(Door * d_ptr);
/**
 * @brief Verifies if the mouse if over a certain door
 * @param  d_ptr     Door object to verify for hovering
 * @param  mouse_bmp Mouse Bitmap (Used for width and height)
 * @param  mouseX    Mouse Current X
 * @param  mouseY    Mouse Current Y
 * @return           Returns true if mouse if over this certain door, or false if not
 */
bool is_mouse_over_door(Door * d_ptr, Bitmap * mouse_bmp, long mouseX, long mouseY);
/**
 * @brief Destructor for the Door Object
 * @param d_ptr Door to destroy
 */
void destroy_door(Door ** d_ptr);

#endif /* __LEVEL_H */
