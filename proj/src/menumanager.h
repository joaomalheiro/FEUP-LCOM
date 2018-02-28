#ifndef __MENU_MANAGER_H
#define __MENU_MANAGER_H

#include "menu.h"
#include "robinix.h"

struct Robinix;

/** @defgroup menumanager menumanager
* @{
*
* Module to manage Menus, storing them and operating over them
*/

typedef enum {
  MAIN_MENU = 0,
  GAME_TYPE_SELECTION,
  LEVEL_SELECTION,
  SCORE_SCREEN,
  N_MENU_TYPES
} menu_types_enum;

typedef struct MenuManager {
  menu_types_enum current_menu;
  Menu * menus[N_MENU_TYPES];
} MenuManager;

/**
 * @brief Creates a MenuManager, that will attempt to load the scores from the scores file into memory
 * @return Returns a pointer to a valid MenuManager or NULL in case of failure
 */
MenuManager * create_menumanager();

/**
 * @brief Destroys the passed MenuManager
 * @param menu_man The MenuManager to destroy
 */
void destroy_menumanager(MenuManager ** menu_man);

/**
 * @brief Updates the current menu mouse_over based on the passed coordinates
 * @param menu_man  MenuManager for which to update mouse_over's
 * @param mouseX   current x of the mouse
 * @param mouseY   current y of the mouse
 */
void menumanager_update_mouse_over(MenuManager * menu_man, long mouseX, long mouseY);

/**
 * @brief Handles mouse clicks on buttons
 * @param menu_man MenuManager in which to process clicks
 * @param rob      Game object that will receive events
 */
void menumanager_handle_button_click(MenuManager * menu_man, struct Robinix * rob);

/**
 * @brief Draws the current Menu
 * @param  menu_man MenuManager to draw
 * @return          1 if it is necessary to draw the High Scores list on screen, 0 otherwise
 */
int draw_menumanager(MenuManager * menu_man);

/**
 * @brief Resets a MenuManager state, by going back to the main menu
 * @param menu_man MenuManager to reset
 */
void reset_menumanager(MenuManager * menu_man);

#endif /* __MENU_MANAGER_H */
