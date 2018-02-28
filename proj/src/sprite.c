#include <stdlib.h>
#include "sprite.h"
#include "bitmap.h"

static void update_sprite(Sprite * s_ptr){

  if(s_ptr->frames_left == 0){
    if (s_ptr->current_bitmap + 1 == s_ptr->n_bitmaps){
      s_ptr->current_bitmap = 0;
    } else {
      s_ptr->current_bitmap = s_ptr->current_bitmap +1;
    }
    s_ptr->frames_left = s_ptr->frames_per_bitmap;
  } else {
    s_ptr->frames_left = s_ptr->frames_left -1;
  }
}

Sprite * create_sprite(char** bmp_paths, int n_bmps, int frames_per_bitmap) {
  //Can't allocate if no bitmaps are passed
  if(n_bmps <= 0) {
    return NULL;
  }

  //Allocating and checking if allocation was successful
  Sprite * s_ptr = malloc(sizeof *s_ptr);

  if(s_ptr == NULL){
    return NULL;
  }

  //Allocating space to store the bmps in the Sprite
  s_ptr->bmps = malloc(n_bmps * sizeof *(s_ptr->bmps));

  //Verifying if the allocation was successful
  if(s_ptr->bmps == NULL){
    //It was not, so deallocate everything allocated so far and return NULL
    free(s_ptr);
    return NULL;
  }

  unsigned int i;
  for (i = 0; i < n_bmps; i++) {
    //Loading the bitmap from the argument passed
    s_ptr->bmps[i] = loadBitmap(bmp_paths[i]);

    //Checking if correctly allocated
    if (s_ptr->bmps[i] == NULL) {

      //If not, destroy the already created members

      //Destroying the already created checkpoints
      int j;
      for(j = 0; j < i; j++) {
        //Deleting
        deleteBitmap(s_ptr->bmps[j]);
        s_ptr->bmps[j] = NULL;
      }

      free(s_ptr->bmps);
      free(s_ptr);
      return NULL;
    }
  }

  s_ptr->frames_per_bitmap = frames_per_bitmap;
  s_ptr->frames_left = frames_per_bitmap;
  s_ptr->n_bitmaps = n_bmps;
  s_ptr->current_bitmap = 0;

  return s_ptr;
}

void destroy_sprite(Sprite ** s_ptr) {

  if(*s_ptr == NULL){
    return;
  }

  //Deallocating the guard checkpoints
  if((*s_ptr)->n_bitmaps > 0){
    int i;
    for(i = 0; i < (*s_ptr)->n_bitmaps; i++){
      deleteBitmap((*s_ptr)->bmps[i]);
    }

    //Deallocating the array itself
    free((*s_ptr)->bmps);
  }
  free(*s_ptr);
  //Setting the pointer to the Sprite struct to NULL so we can know that the object has been deallocated
  //(This is the reason for using a Sprite ** and not a simple Sprite * like in all the other member functions)
  *s_ptr = NULL;
}

void draw_sprite(Sprite * s_ptr, int x, int y) {
    drawBitmap(s_ptr->bmps[s_ptr->current_bitmap], x, y);
    update_sprite(s_ptr);
}

void draw_sprite_wRotation(Sprite * s_ptr, int x, int y, double angle){
  drawBitmapWithRotation(s_ptr->bmps[s_ptr->current_bitmap], x, y, angle);
  update_sprite(s_ptr);
}

Bitmap * sprite_get_current_bitmap(Sprite * s_ptr) {
  return s_ptr->bmps[s_ptr->current_bitmap];
}
