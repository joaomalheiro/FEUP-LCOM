#include "menumanager.h"
#include "robinix.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

MenuManager * create_menumanager() {
  MenuManager * menu_man = malloc(sizeof * menu_man);
  if(menu_man == NULL) {
    return NULL;
  }

  //Allocating submenus
  menu_man->menus[MAIN_MENU] = create_menu(MAIN_MENU);
  menu_man->menus[GAME_TYPE_SELECTION] = create_menu(GAME_TYPE_SELECTION);
  menu_man->menus[LEVEL_SELECTION] = create_menu(LEVEL_SELECTION);
  menu_man->menus[SCORE_SCREEN] = create_menu(SCORE_SCREEN);

  //If creating any of the menus failed, destroy MenuManager and return NULL
  if(menu_man->menus[MAIN_MENU] == NULL || menu_man->menus[GAME_TYPE_SELECTION] == NULL || menu_man->menus[LEVEL_SELECTION] == NULL || menu_man->menus[SCORE_SCREEN] == NULL) {
    printf("DBG: Problem here\n");
    destroy_menumanager(&menu_man);
    return NULL;
  }

  //Setting starting variables
  menu_man->current_menu = MAIN_MENU;

  //MenuManager allocated successfully
  return menu_man;
}

void destroy_menumanager(MenuManager ** menu_man) {
  if(*menu_man == NULL) {
    return;
  }

  //Destroying the menus
  destroy_menu(&((*menu_man)->menus[MAIN_MENU]));
  destroy_menu(&((*menu_man)->menus[GAME_TYPE_SELECTION]));
  destroy_menu(&((*menu_man)->menus[LEVEL_SELECTION]));
  destroy_menu(&((*menu_man)->menus[SCORE_SCREEN]));

  //Destroying the MenuManager
  free(*menu_man);
  *menu_man = NULL;
}

void menumanager_update_mouse_over(MenuManager * menu_man, long mouseX, long mouseY) {
  menu_update_mouse_over(menu_man->menus[menu_man->current_menu], mouseX, mouseY);
}

void menumanager_handle_button_click(MenuManager * menu_man, Robinix * rob) {
  if(menu_man == NULL || rob == NULL) {
    return;
  }

  int click_result = menu_handle_button_click(menu_man->menus[menu_man->current_menu]);

  if(click_result == -1) {
    //No button was clicked
    return;
  }

  //Interpreting click result based on current menu
  switch (menu_man->current_menu) {
    case MAIN_MENU:
      switch(click_result) {
        case 0:
          //Play button
          //Enter level selection menu
          menu_man->current_menu = GAME_TYPE_SELECTION;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[GAME_TYPE_SELECTION]);
          break;
        case 1:
          //High Scores button
          //Enter high scores menu
          menu_man->current_menu = SCORE_SCREEN;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[SCORE_SCREEN]);
          break;
        case 2:
          //Exit game button
          //Send exit game clicked event
          add_event_to_buffer(rob, create_event(CLICKED_EXIT_GAME, 0, 0, '?', NULL));
          break;
        default:
          //No other input expected
          break;
      }
      break;
    case GAME_TYPE_SELECTION:
      switch(click_result) {
        case 0:
          //Solo button
          //Enter level selection menu
          menu_man->current_menu = LEVEL_SELECTION;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[LEVEL_SELECTION]);
          break;
        case 1:
          //Multiplayer button
          //Send clicked multi button event
          add_event_to_buffer(rob, create_event(CLICKED_PLAY_MULTI, 0, 0, '?', NULL));
          break;
        case 2:
          //Back button
          //Go back to main menu
          menu_man->current_menu = MAIN_MENU;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[MAIN_MENU]);
          break;
        default:
          //No other input expected
          break;
      }
      break;
    case LEVEL_SELECTION:
      switch (click_result) {
        case 0:
          //Play level 1 button
          //Send clicked play level 1 button event
          add_event_to_buffer(rob, create_event(CLICKED_PLAY_LVL_1, 0, 0, '?', NULL));
          break;
        case 1:
          //Play level 2 button
          //Send clicked play level 2 button event
          add_event_to_buffer(rob, create_event(CLICKED_PLAY_LVL_2, 0, 0, '?', NULL));
          break;
        case 2:
          //Back button
          //Go back to gametype select menu
          menu_man->current_menu = GAME_TYPE_SELECTION;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[GAME_TYPE_SELECTION]);
      }
      break;
    case SCORE_SCREEN:
      switch (click_result) {
        case 0:
          //Back button
          //Go back to main menu
          menu_man->current_menu = MAIN_MENU;
          //Clear mouse over in menu we are going to change to
          clear_menu_mouse_over(menu_man->menus[MAIN_MENU]);
          break;
      }
      break;
    default:
      break;
  }
}

int draw_menumanager(MenuManager * menu_man) {
  if(menu_man == NULL) {
    return -1;
  }

  draw_menu(menu_man->menus[menu_man->current_menu]);

  //Return 1 to request drawing scores if we are in the score screen
  //(scores are stored externally from the menus so must be drawn like this)
  if(menu_man->current_menu == SCORE_SCREEN) {
    return 1;
  }

  return 0;
}

void reset_menumanager(MenuManager * menu_man) {
  menu_man->current_menu = MAIN_MENU;

  //Since mouse overs are also cleared on menu switch we only need to clear it for the main menu
  clear_menu_mouse_over(menu_man->menus[MAIN_MENU]);
}
