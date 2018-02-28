#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>

#include "sprite.h"
#include "read_xpm.h"
#include "video_gr.h"
#include "video_test.h"

Sprite * create_sprite(char *pic[], int spriteX, int spriteY, int speedX, int speedY) {

  //allocating space for the sprite
  Sprite *sp = (Sprite *) malloc ( sizeof(Sprite));
  //Checking if the memory was allocated correctly
  if( sp == NULL )
    return NULL;

  //reading the sprite pixmap
  sp->map = read_xpm(pic, &(sp->width), &(sp->height));
  //Checking if the reading went through
  if( sp->map == NULL ) {
    free(sp);
    return NULL;
  }

  sp->x = spriteX;
  sp->y = spriteY;
  sp->xspeed = speedX;
  sp->yspeed = speedY;

  //Everything went as expected
  return sp;
}

void destroy_sprite(Sprite *sp) {
  //If sp is not allocated, there is no need to free the memory
  if( sp == NULL )
    return;

  //If sp->map is allocated, free the memory
  if(sp->map)
    free(sp->map);

  free(sp);
  sp = NULL;
}

void draw_sprite(Sprite *sp) {
  int currX = sp->x;
  int currY = sp->y;
  int map_index = 0;
  int maxX = sp->width + sp->x;
  int maxY = sp->height + sp->y;

  for(;currY < maxY; currY++) {
    for(currX = sp->x ; currX < maxX; currX++) {
      vg_fill_pixel(currX, currY, sp->map[map_index]);
      map_index++;
    }
  }
}

void clear_sprite(Sprite *sp) {
  int currX = sp->x;
  int currY = sp->y;
  int map_index = 0;
  int maxX = sp->width + sp->x;
  int maxY = sp->height + sp->y;

  for(;currY < maxY; currY++) {
    for(currX = sp->x ; currX < maxX; currX++) {
      vg_fill_pixel(currX, currY, 0);
      map_index++;
    }
  }

}

int sprite_move(Sprite *sp, int xf, int yf, int nFrames_between_updates) {

  static int nframes_elapsed = 0;

  nframes_elapsed++;

  //If not enough frames have passed by, we do not draw
  if(nframes_elapsed < nFrames_between_updates){
    return 1;
  }

  //Enough frames have passed so we can reset the counter for nframes_elapsed
  nframes_elapsed = 0;

  //Clearing before updating the positions to clear the 'previous position'
  clear_sprite(sp);

  //Updating the position variables
  sp->x += sp->xspeed;
  sp->y += sp->yspeed;

  if(sp->xspeed > 0) {
    if (sp->x >= xf) {
      sp->x = xf;
      sp->y = yf;
      //Drawing the sprite one last time before exiting the drawing loop (returning 0)
      draw_sprite(sp);
      video_dump_fb();
      return 0;
    }
  } else if(sp->yspeed > 0){
    if (sp->y >= yf) {
      sp->x = xf;
      sp->y = yf;
      //Drawing the sprite one last time before exiting the drawing loop (returning 0)
      draw_sprite(sp);
      video_dump_fb();
      return 0;
    }
  } else if (sp->xspeed < 0) {
    if (sp->x <= xf) {
      sp->x = xf;
      sp->y = yf;
      //Drawing the sprite one last time before exiting the drawing loop (returning 0)
      draw_sprite(sp);
      video_dump_fb();
      return 0;
    }
  } else if(sp->yspeed < 0) {
    if (sp->y <= yf) {
      sp->x = xf;
      sp->y = yf;
      //Drawing the sprite one last time before exiting the drawing loop (returning 0)
      draw_sprite(sp);
      video_dump_fb();
      return 0;
    }
  }

  //Default behaviour: The sprite has not finished the movement yet
  draw_sprite(sp);
  video_dump_fb();
  return 1;
}
