#include <stdlib.h>
#include "checkpoint.h"
#include "bitmap.h"

Checkpoint * create_checkpoint(long x, long y, short speed) {

    //Allocating and checking if allocation was successful
    Checkpoint * checkpoint_ptr = malloc(sizeof *checkpoint_ptr);

    if(checkpoint_ptr == NULL){
      return NULL;
    }

    checkpoint_ptr->x = x;
    checkpoint_ptr->y = y;
    checkpoint_ptr->speed = speed;

    //Returning created checkpoint
    return checkpoint_ptr;
}

void destroy_checkpoint(Checkpoint ** checkpoint_ptr) {

  free(*checkpoint_ptr);
  *checkpoint_ptr = NULL;
}
