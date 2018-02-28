#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "guard.h"
#include "bitmap.h"

static bool will_reach_next_checkpoint(Guard * g_ptr, Checkpoint* next_checkpoint) {
  //Analyzing next iteration's x to check if it passes the next checkpoint
  if(g_ptr->speedX > 0) {
    return (g_ptr->guardX + g_ptr->speedX >= next_checkpoint->x);
  } else if(g_ptr->speedX < 0){
    return (g_ptr->guardX + g_ptr->speedX <= next_checkpoint->x);
  }

  //If speedX is 0 then we are moving in the Y direction (guards only move in the cardinal directions)

  //Analyzing next iteration's y to check if it passes the next checkpoint
  if(g_ptr->speedY > 0) {
    return (g_ptr->guardY + g_ptr->speedY >= next_checkpoint->y);
  } else {
    return (g_ptr->guardY + g_ptr->speedY <= next_checkpoint->y);
  }
}

static void update_guard_speed(Guard * g_ptr, Checkpoint * next_checkpoint, Checkpoint * third_checkpoint) {

  //Calculating the speed of the guard
  int speed = next_checkpoint->speed;
  int speedX;
  int speedY;
  int dx = third_checkpoint->x - next_checkpoint->x;
  int dy = third_checkpoint->y - next_checkpoint->y;

  if (abs(dx) > abs(dy)) {
    //The division serves as a way to retrieve the sign of dx
    speedX = speed * (dx / abs(dx));
    speedY = 0;
  } else {
    //The division serves as a way to retrieve the sign of dy
    speedY = speed * (dy / abs(dy));
    speedX = 0;
  }

  g_ptr->speedX = speedX;
  g_ptr->speedY = speedY;

  //Also updating the direction of the guard
  if(g_ptr->speedY < 0){
    //Y is inverted due to drawing more easily0
    g_ptr->currdirection = UP;
  } else if (g_ptr->speedX > 0){
    g_ptr->currdirection = RIGHT;
  } else if (g_ptr->speedY > 0){
    g_ptr->currdirection = DOWN;
  } else if (g_ptr->speedX < 0){
    g_ptr->currdirection = LEFT;
  }
}

static void update_non_cyclical_guard(Guard * g_ptr) {

  //Loading the current and the next checkpoint to get xi,yi,xf,yf
  int next_checkpoint_index;
  Checkpoint* next_checkpoint;
  Checkpoint* third_checkpoint;

  if(g_ptr->goingForward) {
    if (g_ptr->current_checkpoint + 1 == g_ptr->n_checkpoints){
      next_checkpoint_index = g_ptr->current_checkpoint-1;
      g_ptr->goingForward = false;
    } else {
      next_checkpoint_index = g_ptr->current_checkpoint + 1;
    }
  next_checkpoint = g_ptr->checkpoints[next_checkpoint_index];

  } else {
    if (g_ptr->current_checkpoint == 0){
      next_checkpoint_index = g_ptr->current_checkpoint + 1;
      g_ptr->goingForward = true;
    } else {
      next_checkpoint_index = g_ptr->current_checkpoint - 1;
    }
    next_checkpoint = g_ptr->checkpoints[next_checkpoint_index];
  }

  if(will_reach_next_checkpoint(g_ptr, next_checkpoint)){
    int third_checkpoint_index;
    if (g_ptr->goingForward){
      if (next_checkpoint_index + 1 == g_ptr->n_checkpoints){
        third_checkpoint_index = next_checkpoint_index - 1;
      } else {
        third_checkpoint_index = next_checkpoint_index + 1;
      }
    } else {
      if (next_checkpoint_index == 0){
        third_checkpoint_index = next_checkpoint_index + 1;
      } else {
        third_checkpoint_index = next_checkpoint_index - 1;
      }
    }
    third_checkpoint = g_ptr->checkpoints[third_checkpoint_index];

    update_guard_speed(g_ptr, next_checkpoint, third_checkpoint);
    g_ptr->guardX = next_checkpoint->x;
    g_ptr->guardY = next_checkpoint->y;
    g_ptr->current_checkpoint = next_checkpoint_index;
  } else {
    g_ptr->guardX += g_ptr->speedX;
    g_ptr->guardY += g_ptr->speedY;
  }
}

static void update_cyclical_guard(Guard * g_ptr) {

  //Loading the current and the next checkpoint to get xi,yi,xf,yf
  int next_checkpoint_index;
  Checkpoint* next_checkpoint;
  Checkpoint* third_checkpoint;

  if (g_ptr->current_checkpoint + 1 == g_ptr->n_checkpoints){
    next_checkpoint_index = 0;
  } else {
    next_checkpoint_index = g_ptr->current_checkpoint + 1;
  }
  next_checkpoint = g_ptr->checkpoints[next_checkpoint_index];

  if (will_reach_next_checkpoint(g_ptr, next_checkpoint)){
    int third_checkpoint_index;
    if(next_checkpoint_index + 1 == g_ptr->n_checkpoints){
      third_checkpoint_index = 0;
    } else {
      third_checkpoint_index = next_checkpoint_index + 1;
    }
    third_checkpoint = g_ptr->checkpoints[third_checkpoint_index];
    update_guard_speed(g_ptr, next_checkpoint, third_checkpoint);
    g_ptr->guardX = next_checkpoint->x;
    g_ptr->guardY = next_checkpoint->y;
    g_ptr->current_checkpoint = next_checkpoint_index;

  } else {
    g_ptr->guardX += g_ptr->speedX;
    g_ptr->guardY += g_ptr->speedY;
  }
}

Guard * create_guard(Checkpoint checkpoints[], int ncheckpoints, bool isCyclical){

  //Checking if there are at least 2 checkpoints
  if (ncheckpoints < 2){
    return NULL;
  }

  //Allocating and checking if allocation was successful
  Guard * g_ptr = malloc(sizeof *g_ptr);

  if(g_ptr == NULL){
    return NULL;
  }

  ////Bitmap loading
  //Loading guard sprites (one for each direction)
  g_ptr->guardSprites[UP] = loadBitmap("/home/Robinix/res/img/guard/guard_u.bmp");
  g_ptr->guardSprites[RIGHT] = loadBitmap("/home/Robinix/res/img/guard/guard_r.bmp");
  g_ptr->guardSprites[DOWN] = loadBitmap("/home/Robinix/res/img/guard/guard_d.bmp");
  g_ptr->guardSprites[LEFT] = loadBitmap("/home/Robinix/res/img/guard/guard_l.bmp");

  //Checking if the bitmaps were correctly loaded
  if(g_ptr->guardSprites[UP] == NULL || g_ptr->guardSprites[RIGHT] == NULL || g_ptr->guardSprites[DOWN] == NULL || g_ptr->guardSprites[LEFT] == NULL){
    //If any allocation of a bitmap failed, we delete them all to not leave used memory
    deleteBitmap(g_ptr->guardSprites[UP]);
    deleteBitmap(g_ptr->guardSprites[RIGHT]);
    deleteBitmap(g_ptr->guardSprites[DOWN]);
    deleteBitmap(g_ptr->guardSprites[LEFT]);
    free(g_ptr);
    return NULL;
  }

  ////Checkpoints
  g_ptr->n_checkpoints = ncheckpoints;

  int i;
  //Allocating space to store the checkpoints in the Guard
  g_ptr->checkpoints = malloc(ncheckpoints * sizeof *(g_ptr->checkpoints));

  //Verifying if the allocation was successful
  if(g_ptr->checkpoints == NULL){
    //It was not, so deallocate everything allocated so far and return NULL
    deleteBitmap(g_ptr->guardSprites[UP]);
    deleteBitmap(g_ptr->guardSprites[RIGHT]);
    deleteBitmap(g_ptr->guardSprites[DOWN]);
    deleteBitmap(g_ptr->guardSprites[LEFT]);
    free(g_ptr);
    return NULL;
  }

  //Copying the checkpoints to the guard object
  for(i = 0; i < ncheckpoints; i++){
    g_ptr->checkpoints[i] = create_checkpoint(checkpoints[i].x, checkpoints[i].y, checkpoints[i].speed);

    //Checking if correctly allocated
    if(g_ptr->checkpoints[i] == NULL){
      //If not, destroy the already created members

      //Destroying the already created checkpoints
      int j;
      for(j = 0; j < i; j++) {
        destroy_checkpoint(& (g_ptr->checkpoints[j]));
      }

      //Deleting everything else
      deleteBitmap(g_ptr->guardSprites[UP]);
      deleteBitmap(g_ptr->guardSprites[RIGHT]);
      deleteBitmap(g_ptr->guardSprites[DOWN]);
      deleteBitmap(g_ptr->guardSprites[LEFT]);
      free(g_ptr);
      return NULL;
    }
  }

  //Calculating the initial speed of the guard
  int speed = checkpoints[0].speed;
  int speedX;
  int speedY;
  int dx = g_ptr->checkpoints[1]->x - g_ptr->checkpoints[0]->x;
  int dy = g_ptr->checkpoints[1]->y - g_ptr->checkpoints[0]->y;

  if (abs(dx) > abs(dy)) {
    //The division serves as a way to retrieve the sign of dx
    speedX = speed * (dx / abs(dx));
    speedY = 0;
  } else {
    //The division serves as a way to retrieve the sign of dy
    speedY = speed * (dy / abs(dy));
    speedX = 0;
  }

  //Setting the initial speed, starting position, starting checkpoint and starting current direction
  g_ptr->speedX = speedX;
  g_ptr->speedY = speedY;
  g_ptr->guardX = g_ptr->checkpoints[0]->x;
  g_ptr->guardY = g_ptr->checkpoints[0]->y;
  g_ptr->current_checkpoint = 0;

  //Updating starting direction
  if(g_ptr->speedY < 0){
    //Y is inverted due to drawing more easily
    g_ptr->currdirection = UP;
  } else if (g_ptr->speedX > 0){
    g_ptr->currdirection = RIGHT;
  } else if (g_ptr->speedY > 0){
    g_ptr->currdirection = DOWN;
  } else if (g_ptr->speedX < 0){
    g_ptr->currdirection = LEFT;
  }

  //Setting the guard's bool values
  g_ptr->isCyclical = isCyclical;
  g_ptr->goingForward = true;

  //Returning created guard
  return g_ptr;
}

void draw_guard(Guard * g_ptr){
  drawBitmap(g_ptr->guardSprites[g_ptr->currdirection], g_ptr->guardX, g_ptr->guardY);
}

Bitmap * get_guard_current_bitmap(Guard * g_ptr) {
  return g_ptr->guardSprites[g_ptr->currdirection];
}

void update_guard(Guard * g_ptr){
  if (g_ptr->isCyclical) {
    update_cyclical_guard(g_ptr);
  } else {
    update_non_cyclical_guard(g_ptr);
  }
}

void destroy_guard(Guard ** g_ptr) {

  if(*g_ptr == NULL){
    return;
  }

  //Deallocating the guard sprites
  deleteBitmap((*g_ptr)->guardSprites[UP]);
  deleteBitmap((*g_ptr)->guardSprites[RIGHT]);
  deleteBitmap((*g_ptr)->guardSprites[DOWN]);
  deleteBitmap((*g_ptr)->guardSprites[LEFT]);

  //Deallocating the guard checkpoints
  if((*g_ptr)->n_checkpoints > 0){
    int i;
    for(i = 0; i < (*g_ptr)->n_checkpoints; i++){
      destroy_checkpoint(& ((*g_ptr)->checkpoints[i]));
    }

    //Deallocating the array itself
    free((*g_ptr)->checkpoints);
  }

  free(*g_ptr);
  //Setting the pointer to the Guard struct to NULL so we can know that the object has been deallocated
  //(This is the reason for using a Guard ** and not a simple Guard * like in all the other member functions)
  *g_ptr = NULL;
}
