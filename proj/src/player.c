#include <stdlib.h>
#include "player.h"
#include "bitmap.h"

Player * create_player(long startx, long starty, int speedx, int speedy, double angle) {
  //Allocating and checking if allocation was successful
  Player * p_ptr = malloc(sizeof *p_ptr);

  if(p_ptr == NULL){
    return NULL;
  }

  //Loading player sprite
  char * sprite_paths[] = {"/home/Robinix/res/img/player/anim1.bmp", "/home/Robinix/res/img/player/anim2.bmp", "/home/Robinix/res/img/player/anim3.bmp", "/home/Robinix/res/img/player/anim4.bmp", "/home/Robinix/res/img/player/anim5.bmp", "/home/Robinix/res/img/player/anim6.bmp"};
  //Arguments are paths, n_paths and frames per bitmap
  p_ptr->playerSprite = create_sprite(sprite_paths, 6, 10);

  //Checking if correctly loaded
  if(p_ptr->playerSprite == NULL){
    free(p_ptr);
    return NULL;
  }

  //Setting starting values
  p_ptr->isMoving = false;
  p_ptr->x = startx;
  p_ptr->y = starty;
  p_ptr->speedX = speedx;
  p_ptr->speedY = speedy;
  p_ptr->angle = angle;

  //Returning created player
  return p_ptr;
}

void update_player(Player * p_ptr) {
  p_ptr->x += p_ptr->speedX;
  p_ptr->y += p_ptr->speedY;

  if((p_ptr->speedX != 0) || (p_ptr->speedY != 0)){
    p_ptr->isMoving = true;
  } else {
    p_ptr->isMoving = false;
  }

}

void draw_player(Player * p_ptr) {
  if(p_ptr->isMoving){
    draw_sprite_wRotation(p_ptr->playerSprite, p_ptr->x, p_ptr->y, p_ptr->angle);
  } else {
    drawBitmapWithRotation(p_ptr->playerSprite->bmps[0], p_ptr->x, p_ptr->y, p_ptr->angle);
  }
}

Bitmap * get_player_current_bitmap(Player * p_ptr) {
  if(p_ptr->isMoving){
    return sprite_get_current_bitmap(p_ptr->playerSprite);
  } else {
    return p_ptr->playerSprite->bmps[0];
  }
}

void set_player_speed(Player * p_ptr, int speedX, int speedY) {
  p_ptr->speedX = speedX;
  p_ptr->speedY = speedY;
}

void set_player_speedX(Player * p_ptr, int speedX) {
  p_ptr->speedX = speedX;
}

void set_player_speedY(Player * p_ptr, int speedY) {
  p_ptr->speedY = speedY;
}

void set_player_moving_right(Player * p_ptr) {
  p_ptr->speedX = PLAYER_MOVE_SPEED;
}

void set_player_moving_left(Player * p_ptr) {
  p_ptr->speedX = -PLAYER_MOVE_SPEED;
}

void set_player_stopped_x(Player * p_ptr) {
  p_ptr->speedX = 0;
}

void set_player_stopped_y(Player * p_ptr) {
  p_ptr->speedY = 0;
}

void set_player_moving_up(Player * p_ptr) {
  p_ptr->speedY = -PLAYER_MOVE_SPEED;
}

void set_player_moving_down(Player * p_ptr) {
  p_ptr->speedY = PLAYER_MOVE_SPEED;
}

void set_player_angle(Player * p_ptr, double angle) {
  p_ptr->angle = angle;
}

void destroy_player(Player ** p_ptr) {
  if(*p_ptr == NULL){
    return;
  }

  if((*p_ptr)->playerSprite != NULL){
    destroy_sprite(&((*p_ptr)->playerSprite));
  }

  free(*p_ptr);
  //Setting the pointer to the Player struct to NULL so we can know that the object has been deallocated
  //(This is the reason for using a Player ** and not a simple Player * like in all the other member functions)
  *p_ptr = NULL;
}
