#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bitmap.h"

static Menu * create_main_menu() {
  /////Menu configuration variables
  char * background_path = "/home/Robinix/res/img/menu/main/background.bmp";
  unsigned int n_buttons = 3;
  char * button_bmps_paths[] = {"/home/Robinix/res/img/menu/main/play.bmp", "/home/Robinix/res/img/menu/main/highscores.bmp", "/home/Robinix/res/img/menu/main/exit.bmp"};
  char * button_hovered_bmps_paths[] = {"/home/Robinix/res/img/menu/main/play_hover.bmp", "/home/Robinix/res/img/menu/main/highscores_hover.bmp", "/home/Robinix/res/img/menu/main/exit_hover.bmp"};
  long button_x[] = {350, 350, 350};
  long button_y[] = {270, 390, 510};
  /////

  //Allocating menu object
  Menu * menu = malloc(sizeof * menu);
  if(menu == NULL) {
    return NULL;
  }

  //Loading background bitmap into memory
  menu->background = loadBitmap(background_path);
  if(menu->background == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Creating buttons

  //Allocating space for the array
  menu->n_buttons = n_buttons;
  menu->buttons = calloc(menu->n_buttons, sizeof *menu->buttons);
  if(menu->buttons == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Allocating each button
  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    menu->buttons[i] = create_button(button_x[i], button_y[i], button_bmps_paths[i], button_hovered_bmps_paths[i]);
    if(menu->buttons[i] == NULL) {
      destroy_menu(&menu);
      return NULL;
    }
  }

  //Menu allocation done
  return menu;
}

static Menu * create_gametype_menu() {
  /////Menu configuration variables
  char * background_path = "/home/Robinix/res/img/menu/gametype/background.bmp";
  unsigned int n_buttons = 3;
  char * button_bmps_paths[] = {"/home/Robinix/res/img/menu/gametype/solo.bmp", "/home/Robinix/res/img/menu/gametype/multi.bmp", "/home/Robinix/res/img/menu/gametype/back.bmp"};
  char * button_hovered_bmps_paths[] = {"/home/Robinix/res/img/menu/gametype/solo_hover.bmp", "/home/Robinix/res/img/menu/gametype/multi_hover.bmp", "/home/Robinix/res/img/menu/gametype/back_hover.bmp"};
  long button_x[] = {350, 350, 350};
  long button_y[] = {270, 390, 510};
  /////

  //Allocating menu object
  Menu * menu = malloc(sizeof * menu);
  if(menu == NULL) {
    return NULL;
  }

  //Loading background bitmap into memory
  menu->background = loadBitmap(background_path);
  if(menu->background == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Creating buttons

  //Allocating space for the array
  menu->n_buttons = n_buttons;
  menu->buttons = calloc(menu->n_buttons, sizeof *menu->buttons);
  if(menu->buttons == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Allocating each button
  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    menu->buttons[i] = create_button(button_x[i], button_y[i], button_bmps_paths[i], button_hovered_bmps_paths[i]);
    if(menu->buttons[i] == NULL) {
      destroy_menu(&menu);
      return NULL;
    }
  }

  //Menu allocation done
  return menu;
}

static Menu * create_level_menu() {
  /////Menu configuration variables
  char * background_path = "/home/Robinix/res/img/menu/levels/background.bmp";
  unsigned int n_buttons = 3;
  char * button_bmps_paths[] = {"/home/Robinix/res/img/menu/levels/play1.bmp", "/home/Robinix/res/img/menu/levels/play2.bmp", "/home/Robinix/res/img/menu/levels/back.bmp"};
  char * button_hovered_bmps_paths[] = {"/home/Robinix/res/img/menu/levels/play1_hover.bmp", "/home/Robinix/res/img/menu/levels/play2_hover.bmp", "/home/Robinix/res/img/menu/levels/back_hover.bmp"};
  long button_x[] = {350, 350, 350};
  long button_y[] = {270, 390, 510};
  /////

  //Allocating menu object
  Menu * menu = malloc(sizeof * menu);
  if(menu == NULL) {
    return NULL;
  }

  //Loading background bitmap into memory
  menu->background = loadBitmap(background_path);
  if(menu->background == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Creating buttons

  //Allocating space for the array
  menu->n_buttons = n_buttons;
  menu->buttons = calloc(menu->n_buttons, sizeof *menu->buttons);
  if(menu->buttons == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Allocating each button
  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    menu->buttons[i] = create_button(button_x[i], button_y[i], button_bmps_paths[i], button_hovered_bmps_paths[i]);
    if(menu->buttons[i] == NULL) {
      destroy_menu(&menu);
      return NULL;
    }
  }

  //Menu allocation done
  return menu;
}

static Menu * create_highscore_menu() {
  /////Menu configuration variables
  char * background_path = "/home/Robinix/res/img/menu/highscores/background.bmp";
  unsigned int n_buttons = 1;
  char * button_bmps_paths[] = {"/home/Robinix/res/img/menu/highscores/back.bmp"};
  char * button_hovered_bmps_paths[] = {"/home/Robinix/res/img/menu/highscores/back_hover.bmp"};
  long button_x[] = {660};
  long button_y[] = {645};
  /////

  //Allocating menu object
  Menu * menu = malloc(sizeof * menu);
  if(menu == NULL) {
    return NULL;
  }

  //Loading background bitmap into memory
  menu->background = loadBitmap(background_path);
  if(menu->background == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Creating buttons

  //Allocating space for the array
  menu->n_buttons = n_buttons;
  menu->buttons = calloc(menu->n_buttons, sizeof *menu->buttons);
  if(menu->buttons == NULL) {
    destroy_menu(&menu);
    return NULL;
  }

  //Allocating each button
  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    menu->buttons[i] = create_button(button_x[i], button_y[i], button_bmps_paths[i], button_hovered_bmps_paths[i]);
    if(menu->buttons[i] == NULL) {
      destroy_menu(&menu);
      return NULL;
    }
  }

  //Menu allocation done
  return menu;
}

Menu * create_menu(int menu_id) {
  switch(menu_id) {
    case 0:
      return create_main_menu();
      break;
    case 1:
      return create_gametype_menu();
      break;
    case 2:
      return create_level_menu();
      break;
    case 3:
      return create_highscore_menu();
      break;
    default:
      return NULL;
      break;
  }
}

void menu_update_mouse_over(Menu * menu, long mouseX, long mouseY) {
  if(menu == NULL) {
    return;
  }

  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    if(is_mouse_over_button(menu->buttons[i], mouseX, mouseY)) {
      menu->buttons[i]->is_hovered = true;
    } else {
      menu->buttons[i]->is_hovered = false;
    }
  }
}

void clear_menu_mouse_over(Menu * menu) {
  if(menu == NULL) {
    return;
  }

  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    menu->buttons[i]->is_hovered = false;
  }
}

int menu_handle_button_click(Menu * menu) {
  if(menu == NULL) {
    return -1;
  }

  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    if(menu->buttons[i]->is_hovered) {
      //If a button was hovered when a click event was fired then it was clicked
      return i;
    }
  }

  return -1;
}

static void draw_menu_background(Menu * menu) {
  if(menu == NULL) {
    return;
  }

  //Because the background will basically always be fullscreen (and since it is the background itself, transparency doesn't matter)
  drawFullscreenBitmap(menu->background);
}

void draw_menu(Menu * menu) {
  if(menu == NULL) {
    return;
  }

  draw_menu_background(menu);

  int i;
  for(i = 0; i < menu->n_buttons; i++) {
    draw_button(menu->buttons[i]);
  }
}

void destroy_menu(Menu ** menu) {
  if(*menu == NULL) {
    return;
  }

  deleteBitmap((*menu)->background);

  int i;
  for(i = 0; i < (*menu)->n_buttons; i++) {
    destroy_button(&((*menu)->buttons[i]));
  }
  //Deallocating the array itself
  free((*menu)->buttons);

  free(*menu);
  *menu = NULL;
}

Button * create_button(int x, int y, char * bmp_path, char * hovered_bmp_path) {
  Button * but = malloc(sizeof * but);

  if(but == NULL) {
    return NULL;
  }

  //Loading button bmps

  but->bmp = loadBitmap(bmp_path);
  if(but->bmp == NULL) {
    destroy_button(&but);
    return NULL;
  }

  but->hovered_bmp = loadBitmap(hovered_bmp_path);
  if(but->hovered_bmp == NULL) {
    destroy_button(&but);
    return NULL;
  }

  //Setting internal values
  but->x = x;
  but->y = y;
  but->is_hovered = false;

  return but;
}

bool is_mouse_over_button(Button * but, long mouseX, long mouseY) {
  //AABB collision is enough for this
  //mouse x is between x and X AND mouse y is between y and Y, where lowercase letter is the side start and uppercase is side end
  return ((mouseX >= but->x) && (mouseX <= but->x + but->bmp->bitmapInfoHeader.width)) && ((mouseY >= but->y) && (mouseY <= but->y + but->bmp->bitmapInfoHeader.height));
}

void draw_button(Button * but) {
  if(but == NULL) {
    return;
  }

  if(but->is_hovered) {
    drawBitmap(but->hovered_bmp, but->x, but->y);
  } else {
    drawBitmap(but->bmp, but->x, but->y);
  }
}

void destroy_button(Button ** but) {
  if(*but == NULL) {
    return;
  }

  deleteBitmap((*but)->bmp);
  deleteBitmap((*but)->hovered_bmp);

  free(*but);
  *but = NULL;
}
