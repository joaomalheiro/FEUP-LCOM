#ifndef __MENU_H
#define __MENU_H

#include <stdbool.h>
#include "bitmap.h"

/** @defgroup menu menu
 * @{
 *
 * Module that represents a Menu, allowing for operations over it
 */

typedef struct {
  long x;
  long y;
  Bitmap * bmp;
  Bitmap * hovered_bmp;
  bool is_hovered;
} Button;

typedef struct {
  Bitmap * background;
  Button ** buttons;
  unsigned int n_buttons;
} Menu;

/**
 * @brief Menu Object Constructor, creates a menu based on the passed ID
 * @param  menu_id Menu ID to construct
 * @return         Returns a pointer to a valid Menu Object or NULL in case of failure
 */
Menu * create_menu(int menu_id);

/**
 * @brief Updates mouse over for a Menu Object
 * @param menu   Menu to update mouse over for
 * @param mouseX Current Mouse X
 * @param mouseY Current Mouse Y
 */
void menu_update_mouse_over(Menu * menu, long mouseX, long mouseY);

/**
 * @brief Clears mouse overs for all Menu buttons
 * @param menu Menu to clear mouse over for
 */
void clear_menu_mouse_over(Menu * menu);

/**
 * @brief Handles button click in a Menu
 * @param  menu Menu to handle button clicks in
 * @return      Returns the index of the button that was clicked or -1 if none was clicked
 */
int menu_handle_button_click(Menu * menu);

/**
 * @brief Draws a Menu
 * @param menu Menu to draw
 */
void draw_menu(Menu * menu);

/**
 * @brief Menu Object Destructor
 * @param menu Menu to destroy
 */
void destroy_menu(Menu ** menu);

///Button Object

/**
 * @brief Button Object Constructor
 * @param  x                X to create the Button in
 * @param  y                Y to create the Button in
 * @param  bmp_path         Path to the Button bmp
 * @param  hovered_bmp_path Path to the Button Hovered bmp
 * @return                  Returns a pointer to a valid Button Object or NULL in case of failure
 */
Button * create_button(int x, int y, char * bmp_path, char * hovered_bmp_path);

/**
 * @brief Decides if the mouse is over a Button or not, for use with mouse hovering
 * @param  but    Button the check for mouse "collision"
 * @param  mouseX Current Mouse X
 * @param  mouseY Current Mouse Y
 * @return        Returns true if the mouse is over the Button, or false if it is not
 */
bool is_mouse_over_button(Button * but, long mouseX, long mouseY);

/**
 * @brief Draws a Button
 * @param but Button to draw
 */
void draw_button(Button * but);

/**
 * @brief Button Object Destructor
 * @param but Button Object to destroy
 */
void destroy_button(Button ** but);

#endif /* __MENU_H */
