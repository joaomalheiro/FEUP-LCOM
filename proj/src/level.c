#include "level.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <string.h>
#include "checkpoint.h"
#include "guard.h"
#include "player.h"
#include "robinix.h"
#include "video_gr.h" /* For getting resolutions */

//Receives x and y variables and width and height and makes sure that they are inside the screen
static void limit_xy_inside_screen(long * x, long * y, int width, int height) {
  //NOTE: x and y are centered in the top left corner, thus the verifications are done like so

  if(*x <= 0) {
    *x = 0;
  } else if(*x + width >= getHorResolution()) {
    *x = getHorResolution() - width;
  }

  if(*y <= 0) {
    *y = 0;
  } else if(*y + height >= getVerResolution()) {
    *y = getVerResolution() - height;
  }
}

////Level loading

static Level * load_level_0() {
  //Test level, does not allocate coins or treasure, for simplicity's sake
  ////Level configuration variables
  long player_start_x = 200;
  long player_start_y = 500;
  unsigned int n_guards = 2;
  int n_checkpoints[] = {4, 3};
  Checkpoint checkpoints_g1[] = {{.x = 100, .y = 100, .speed = 2},
                                 {.x = 800, .y = 100, .speed = 3},
                                 {.x = 800, .y = 500, .speed = 4},
                                 {.x = 100, .y = 500, .speed = 5}};
  Checkpoint checkpoints_g2[] = {{.x = 200, .y = 200, .speed = 1},
                                 {.x = 600, .y = 200, .speed = 6},
                                 {.x = 600, .y = 400, .speed = 3}};
  //It is not possible to delcare a bidimensional array easily, so we have to resort to several unidimensional ones...

  bool is_cyclical[] = {true, false};
  char * background_path = "/home/Robinix/res/img/backgrounds/background_play.bmp";
  char * wall_path = "/home/Robinix/res/img/levels/walls_test.bmp";
  ////

  //Allocating level object
  Level * l_ptr = calloc(1, sizeof *l_ptr);

  if(l_ptr == NULL) {
    return NULL;
  }

  //Creating player (with starting coordinates and 0 speeds and starting angle)
  l_ptr->player = create_player(player_start_x, player_start_y, 0, 0, 0);

  if(l_ptr->player == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating guards
  l_ptr->n_guards = n_guards;
  l_ptr->guards = calloc(n_guards, sizeof * l_ptr->guards);

  l_ptr->guards[0] = create_guard(checkpoints_g1, n_checkpoints[0], is_cyclical[0]);
  if(l_ptr->guards[0] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[1] = create_guard(checkpoints_g2, n_checkpoints[1], is_cyclical[1]);
  if(l_ptr->guards[1] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading background bitmap
  l_ptr->background_bmp = loadBitmap(background_path);

  if(l_ptr->background_bmp == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading walls bitmap
  l_ptr->level_walls = loadBitmap(wall_path);

  if(l_ptr->level_walls == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Ensuring everything else is set to NULL or 0 to make sure there are no problems
  l_ptr->treasure = NULL;
  l_ptr->n_coins = 0;
  l_ptr->coins = NULL;
  l_ptr->exit = NULL;

  return l_ptr;
}

static Level * load_level_1() {
  ////Level configuration variables
  long player_start_x = 450;
  long player_start_y = 700;
  unsigned int n_guards = 6;
  int n_checkpoints[] = {6, 3, 4, 3, 3, 2};
  Checkpoint checkpoints_g1[] = {{.x = 145, .y = 670, .speed = 2},
                                 {.x = 10 , .y = 670, .speed = 3},
                                 {.x = 10 , .y = 10 , .speed = 4},
                                 {.x = 775, .y = 10 , .speed = 5},
                                 {.x = 775, .y = 80 , .speed = 5},
                                 {.x = 145, .y = 80 , .speed = 5}};
  Checkpoint checkpoints_g2[] = {{.x = 775, .y = 50 , .speed = 4},
                                 {.x = 75 , .y = 50 , .speed = 6},
                                 {.x = 75 , .y = 670, .speed = 3}};
  Checkpoint checkpoints_g3[] = {{.x = 830, .y = 130, .speed = 7},
                                 {.x = 980, .y = 130, .speed = 8},
                                 {.x = 980, .y = 670, .speed = 8},
                                 {.x = 830, .y = 670, .speed = 6}};
  Checkpoint checkpoints_g4[] = {{.x = 225, .y = 530, .speed = 4},
                                 {.x = 225, .y = 170, .speed = 6},
                                 {.x = 740, .y = 170, .speed = 3}};
  Checkpoint checkpoints_g5[] = {{.x = 310, .y = 510, .speed = 4},
                                 {.x = 310, .y = 250, .speed = 6},
                                 {.x = 740, .y = 250, .speed = 3}};
  Checkpoint checkpoints_g6[] = {{.x = 580, .y = 300, .speed = 4},
                                 {.x = 580, .y = 490, .speed = 6}};


  //It is not possible to declare a bidimensional array easily, so we have to resort to several unidimensional ones...
  bool is_cyclical[] = {true, false, true, false, false, true};
  char * background_path = "/home/Robinix/res/img/backgrounds/level_1_ground.bmp";
  char * wall_path = "/home/Robinix/res/img/levels/level_1_map.bmp";
  unsigned int n_coins = 5;
  int coin_x[] = {10, 250, 500 , 590 , 900};
  int coin_y[] = {25, 375, 495 , 350, 50};
  int treasure_x = 512;
  int treasure_y = 384;
  int exit_x = 475;
  int exit_y = 730;
  ////

  //Allocating level object
  Level * l_ptr = calloc(1, sizeof *l_ptr);

  if(l_ptr == NULL) {
    return NULL;
  }

  //Creating player (with starting coordinates and 0 speeds and starting angle)
  l_ptr->player = create_player(player_start_x, player_start_y, 0, 0, 0);

  if(l_ptr->player == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating guards
  l_ptr->n_guards = n_guards;
  l_ptr->guards = calloc(n_guards, sizeof * l_ptr->guards);

  l_ptr->guards[0] = create_guard(checkpoints_g1, n_checkpoints[0], is_cyclical[0]);
  if(l_ptr->guards[0] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[1] = create_guard(checkpoints_g2, n_checkpoints[1], is_cyclical[1]);
  if(l_ptr->guards[1] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[2] = create_guard(checkpoints_g3, n_checkpoints[2], is_cyclical[2]);
  if(l_ptr->guards[2] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[3] = create_guard(checkpoints_g4, n_checkpoints[3], is_cyclical[3]);
  if(l_ptr->guards[3] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[4] = create_guard(checkpoints_g5, n_checkpoints[4], is_cyclical[4]);
  if(l_ptr->guards[4] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[5] = create_guard(checkpoints_g6, n_checkpoints[5], is_cyclical[5]);
  if(l_ptr->guards[5] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating treasure
  l_ptr->treasure = create_treasure(treasure_x, treasure_y);

  if(l_ptr->treasure == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating coins
  l_ptr->n_coins = n_coins;
  l_ptr->coins = calloc(n_coins, sizeof * l_ptr->coins);
  int i;
  for(i = 0; i < l_ptr->n_coins; i++) {
    l_ptr->coins[i] = create_coin(coin_x[i], coin_y[i]);
    if(l_ptr->coins[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Creating exit
  l_ptr->exit = create_exit(exit_x, exit_y, false);

  if(l_ptr->exit == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading background bitmap
  l_ptr->background_bmp = loadBitmap(background_path);

  if(l_ptr->background_bmp == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading walls bitmap
  l_ptr->level_walls = loadBitmap(wall_path);

  if(l_ptr->level_walls == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Ensuring everything else is set to NULL or 0 to make sure there are no problems
  l_ptr->doors = NULL;
  l_ptr->n_doors = 0;

  //Level creation successful
  return l_ptr;
}

static Level * load_level_2() {
  ////Level configuration variables
  long player_start_x = 10;
  long player_start_y = 400;
  unsigned int n_guards = 6;
  int n_checkpoints[] = {4,4,4,4,4,6};
  Checkpoint checkpoints_g1[] = {{.x = 10 , .y = 290, .speed = 4},
                                 {.x = 250, .y = 290, .speed = 3},
                                 {.x = 250, .y = 155, .speed = 4},
                                 {.x = 10 , .y = 155, .speed = 5}};
  Checkpoint checkpoints_g2[] = {{.x = 10 , .y = 500, .speed = 3},
                                 {.x = 250, .y = 500, .speed = 4},
                                 {.x = 250, .y = 610, .speed = 3},
                                 {.x = 10 , .y = 610, .speed = 5}};
  Checkpoint checkpoints_g3[] = {{.x = 330, .y = 290, .speed = 3},
                                 {.x = 700, .y = 290, .speed = 4},
                                 {.x = 700, .y = 155, .speed = 3},
                                 {.x = 330, .y = 155, .speed = 4}};
  Checkpoint checkpoints_g4[] = {{.x = 330, .y = 500, .speed = 5},
                                 {.x = 700, .y = 500, .speed = 4},
                                 {.x = 700, .y = 610, .speed = 5},
                                 {.x = 330, .y = 610, .speed = 3}};
  Checkpoint checkpoints_g5[] = {{.x = 10 , .y = 360, .speed = 5},
                                 {.x = 700, .y = 360, .speed = 4},
                                 {.x = 700, .y = 410, .speed = 3},
                                 {.x = 10 , .y = 410, .speed = 4}};

  Checkpoint checkpoints_g6[] = {{.x = 775 , .y = 155, .speed = 5},
                                 {.x = 960, .y = 155, .speed = 4},
                                 {.x = 960, .y = 430, .speed = 3},
                                 {.x = 775 , .y = 430, .speed = 5},
                                 {.x = 775 , .y = 610, .speed = 4},
                                 {.x = 960 , .y = 610, .speed = 4}};


  //It is not possible to declare a bidimensional array easily, so we have to resort to several unidimensional ones...
  bool is_cyclical[] = {true, true, true, true, true, false};
  char * background_path = "/home/Robinix/res/img/backgrounds/level_2_ground.bmp";
  char * wall_path = "/home/Robinix/res/img/levels/level_2_map.bmp";
  unsigned int n_coins = 5;
  int coin_x[] = {10, 250, 500, 10, 275};
  int coin_y[] = {25, 375, 495, 200 , 500};
  int treasure_x = 690;
  int treasure_y = 160;
  int exit_x = 100;
  int exit_y = 100;
  unsigned int n_doors = 5;
  int door_x[] = {118, 118, 502, 502, 454};
  int door_y[] = {328, 472, 328, 472, 650};
  bool door_start_closed[] = {true, true, true, true, true};
  ////

  //Allocating level object
  Level * l_ptr = calloc(1, sizeof *l_ptr);

  if(l_ptr == NULL) {
    return NULL;
  }

  //Creating player (with starting coordinates and 0 speeds and starting angle)
  l_ptr->player = create_player(player_start_x, player_start_y, 0, 0, 0);

  if(l_ptr->player == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating guards
  l_ptr->n_guards = n_guards;
  l_ptr->guards = calloc(n_guards, sizeof * l_ptr->guards);

  l_ptr->guards[0] = create_guard(checkpoints_g1, n_checkpoints[0], is_cyclical[0]);
  if(l_ptr->guards[0] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[1] = create_guard(checkpoints_g2, n_checkpoints[1], is_cyclical[1]);
  if(l_ptr->guards[1] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[2] = create_guard(checkpoints_g3, n_checkpoints[2], is_cyclical[2]);
  if(l_ptr->guards[2] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[3] = create_guard(checkpoints_g4, n_checkpoints[3], is_cyclical[3]);
  if(l_ptr->guards[3] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[4] = create_guard(checkpoints_g5, n_checkpoints[4], is_cyclical[4]);
  if(l_ptr->guards[4] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[5] = create_guard(checkpoints_g6, n_checkpoints[5], is_cyclical[5]);
  if(l_ptr->guards[5] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating treasure
  l_ptr->treasure = create_treasure(treasure_x, treasure_y);

  if(l_ptr->treasure == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating coins
  l_ptr->n_coins = n_coins;
  l_ptr->coins = calloc(n_coins, sizeof * l_ptr->coins);
  int i;
  for(i = 0; i < l_ptr->n_coins; i++) {
    l_ptr->coins[i] = create_coin(coin_x[i], coin_y[i]);
    if(l_ptr->coins[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Creating exit
  l_ptr->exit = create_exit(exit_x, exit_y, false);

  if(l_ptr->exit == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating doors
  l_ptr->n_doors = n_doors;
  l_ptr->doors = calloc(n_doors, sizeof * l_ptr->doors);
  //No need to redeclare i, declared above
  for(i = 0; i < l_ptr->n_doors; i++) {
    l_ptr->doors[i] = create_door(door_x[i], door_y[i], door_start_closed[i]);
    if(l_ptr->doors[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Loading background bitmap
  l_ptr->background_bmp = loadBitmap(background_path);

  if(l_ptr->background_bmp == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading walls bitmap
  l_ptr->level_walls = loadBitmap(wall_path);

  if(l_ptr->level_walls == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Level creation successful
  return l_ptr;
}

static Level * load_level_mp1() {
  ////Level configuration variables
  long player_start_x = 10;
  long player_start_y = 10;
  unsigned int n_guards = 4;
  int n_checkpoints[] = {4,4,4,4};
  Checkpoint checkpoints_g1[] = {{.x = 170, .y = 200, .speed = 4},
                                 {.x = 170, .y = 500, .speed = 6},
                                 {.x = 40 , .y = 500, .speed = 4},
                                 {.x = 40 , .y = 200, .speed = 6}};
  Checkpoint checkpoints_g2[] = {{.x = 430, .y = 200, .speed = 4},
                                 {.x = 430, .y = 500, .speed = 6},
                                 {.x = 300, .y = 500, .speed = 4},
                                 {.x = 300, .y = 200, .speed = 6}};

  Checkpoint checkpoints_g3[] = {{.x = 600, .y = 200, .speed = 6},
                                 {.x =1000, .y = 200, .speed = 4},
                                 {.x =1000, .y = 300, .speed = 6},
                                 {.x =600 , .y = 300, .speed = 4}};
  Checkpoint checkpoints_g4[] = {{.x = 600, .y = 400, .speed = 6},
                                 {.x =1000, .y = 400, .speed = 4},
                                 {.x =1000, .y = 500, .speed = 6},
                                 {.x =600 , .y = 500, .speed = 4}};


  //It is not possible to declare a bidimensional array easily, so we have to resort to several unidimensional ones...
  bool is_cyclical[] = {true, true, true, true};
  char * background_path = "/home/Robinix/res/img/backgrounds/level_mp1_ground.bmp";
  char * wall_path = "/home/Robinix/res/img/levels/level_mp_map1.bmp";
  unsigned int n_coins = 5;
  int coin_x[] = {970, 970,970, 20, 970};
  int coin_y[] = {50, 140, 450, 515 , 570};
  int treasure_x = 900;
  int treasure_y = 700;
  int exit_x = 25;
  int exit_y = 700;
  unsigned int n_doors = 4;
  int door_x[] = {60 , 330, 300, 570};
  int door_y[] = {170, 170, 535, 535};
  bool door_start_closed[] = {true, true, true, true};
  ////

  //Allocating level object
  Level * l_ptr = calloc(1, sizeof *l_ptr);

  if(l_ptr == NULL) {
    return NULL;
  }

  //Creating player (with starting coordinates and 0 speeds and starting angle)
  l_ptr->player = create_player(player_start_x, player_start_y, 0, 0, 0);

  if(l_ptr->player == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating guards
  l_ptr->n_guards = n_guards;
  l_ptr->guards = calloc(n_guards, sizeof * l_ptr->guards);

  l_ptr->guards[0] = create_guard(checkpoints_g1, n_checkpoints[0], is_cyclical[0]);
  if(l_ptr->guards[0] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[1] = create_guard(checkpoints_g2, n_checkpoints[1], is_cyclical[1]);
  if(l_ptr->guards[1] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[2] = create_guard(checkpoints_g3, n_checkpoints[2], is_cyclical[2]);
  if(l_ptr->guards[2] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[3] = create_guard(checkpoints_g4, n_checkpoints[3], is_cyclical[3]);
  if(l_ptr->guards[3] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating treasure
  l_ptr->treasure = create_treasure(treasure_x, treasure_y);

  if(l_ptr->treasure == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating coins
  l_ptr->n_coins = n_coins;
  l_ptr->coins = calloc(n_coins, sizeof * l_ptr->coins);
  int i;
  for(i = 0; i < l_ptr->n_coins; i++) {
    l_ptr->coins[i] = create_coin(coin_x[i], coin_y[i]);
    if(l_ptr->coins[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Creating exit
  l_ptr->exit = create_exit(exit_x, exit_y, true);
  if(l_ptr->exit == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating doors
  l_ptr->n_doors = n_doors;
  l_ptr->doors = calloc(n_doors, sizeof * l_ptr->doors);
  //No need to redeclare i, declared above
  for(i = 0; i < l_ptr->n_doors; i++) {
    l_ptr->doors[i] = create_door(door_x[i], door_y[i], door_start_closed[i]);
    if(l_ptr->doors[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Loading background bitmap
  l_ptr->background_bmp = loadBitmap(background_path);

  if(l_ptr->background_bmp == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading walls bitmap
  l_ptr->level_walls = loadBitmap(wall_path);

  if(l_ptr->level_walls == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Level creation successful
  return l_ptr;
}

static Level * load_level_mp2() {
  ////Level configuration variables
  long player_start_x = 920;
  long player_start_y = 10;
  unsigned int n_guards = 4;
  int n_checkpoints[] = {4,4,4,4};
  Checkpoint checkpoints_g1[] = {{.x = 870, .y = 490, .speed = 4},
                                 {.x = 870, .y = 210, .speed = 6},
                                 {.x = 980, .y = 210, .speed = 4},
                                 {.x = 980, .y = 490, .speed = 6}};
  Checkpoint checkpoints_g2[] = {{.x = 590, .y = 490, .speed = 4},
                                 {.x = 590, .y = 210, .speed = 6},
                                 {.x = 720, .y = 210, .speed = 4},
                                 {.x = 720, .y = 490, .speed = 6}};

  Checkpoint checkpoints_g3[] = {{.x = 420, .y = 490, .speed = 6},
                                 {.x = 24 , .y = 490, .speed = 4},
                                 {.x = 24 , .y = 410, .speed = 6},
                                 {.x = 420, .y = 410, .speed = 4}};
  Checkpoint checkpoints_g4[] = {{.x = 420, .y = 310, .speed = 6},
                                 {.x = 24 , .y = 310, .speed = 4},
                                 {.x = 24 , .y = 210, .speed = 6},
                                 {.x = 420, .y = 210, .speed = 4}};


  //It is not possible to declare a bidimensional array easily, so we have to resort to several unidimensional ones...
  bool is_cyclical[] = {true, true, true, true};
  char * background_path = "/home/Robinix/res/img/backgrounds/level_mp2_ground.bmp";
  char * wall_path = "/home/Robinix/res/img/levels/level_mp_map2.bmp";
  unsigned int n_coins = 5;
  int coin_x[] = {25, 25,25, 1000, 25};
  int coin_y[] = {20, 140, 220, 350 , 520};
  int treasure_x = 900;
  int treasure_y = 700;
  int exit_x = 25;
  int exit_y = 700;
  unsigned int n_doors = 4;
  int door_x[] = {875 , 604, 634, 364};
  int door_y[] = {170, 170, 535, 535};
  bool door_start_closed[] = {true, true, true, true};
  ////

  //Allocating level object
  Level * l_ptr = calloc(1, sizeof *l_ptr);

  if(l_ptr == NULL) {
    return NULL;
  }

  //Creating player (with starting coordinates and 0 speeds and starting angle)
  l_ptr->player = create_player(player_start_x, player_start_y, 0, 0, 0);

  if(l_ptr->player == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating guards
  l_ptr->n_guards = n_guards;
  l_ptr->guards = calloc(n_guards, sizeof * l_ptr->guards);

  l_ptr->guards[0] = create_guard(checkpoints_g1, n_checkpoints[0], is_cyclical[0]);
  if(l_ptr->guards[0] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[1] = create_guard(checkpoints_g2, n_checkpoints[1], is_cyclical[1]);
  if(l_ptr->guards[1] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[2] = create_guard(checkpoints_g3, n_checkpoints[2], is_cyclical[2]);
  if(l_ptr->guards[2] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  l_ptr->guards[3] = create_guard(checkpoints_g4, n_checkpoints[3], is_cyclical[3]);
  if(l_ptr->guards[3] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating treasure
  l_ptr->treasure = create_treasure(treasure_x, treasure_y);

  if(l_ptr->treasure == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating coins
  l_ptr->n_coins = n_coins;
  l_ptr->coins = calloc(n_coins, sizeof * l_ptr->coins);
  int i;
  for(i = 0; i < l_ptr->n_coins; i++) {
    l_ptr->coins[i] = create_coin(coin_x[i], coin_y[i]);
    if(l_ptr->coins[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Creating exit
  l_ptr->exit = create_exit(exit_x, exit_y, true);

  if(l_ptr->exit == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Creating doors
  l_ptr->n_doors = n_doors;
  l_ptr->doors = calloc(n_doors, sizeof * l_ptr->doors);
  //No need to redeclare i, declared above
  for(i = 0; i < l_ptr->n_doors; i++) {
    l_ptr->doors[i] = create_door(door_x[i], door_y[i], door_start_closed[i]);
    if(l_ptr->doors[i] == NULL) {
      destroy_level(&l_ptr);
      return NULL;
    }
  }

  //Loading background bitmap
  l_ptr->background_bmp = loadBitmap(background_path);

  if(l_ptr->background_bmp == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Loading walls bitmap
  l_ptr->level_walls = loadBitmap(wall_path);

  if(l_ptr->level_walls == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Level creation successful
  return l_ptr;
}

Level * create_level(int level_n, bool is_mp) {
  Level * l_ptr = NULL;

  switch(level_n) {
    case 0:
      l_ptr = load_level_0();
      break;
    case 1:
      if(is_mp) {
        l_ptr = load_level_mp1();
      } else {
        l_ptr = load_level_1();
      }
      break;
    case 2:
      if(is_mp) {
        l_ptr = load_level_mp2();
      } else {
        l_ptr = load_level_2();
      }
      break;
    default:
      return NULL;
      break;
  }

  if(l_ptr == NULL) {
    return NULL;
  }

  ////Some allocations are always the same so they can be done here

  //Loading the bitmap of the level border into memory
  l_ptr->level_border = loadBitmap("/home/Robinix/res/img/other/level_border.bmp");

  //If the level border bmp was not correctly allocated, destroy the level and return NULL
  if(l_ptr->level_border == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }


  //Loading the bitmaps of the mouse pointers into memory
  l_ptr->mouse_bmps[M_OVER_NOTHING] = loadBitmap("/home/Robinix/res/img/mouse/mouse_arrow.bmp");
  l_ptr->mouse_bmps[M_OVER_DOOR] = loadBitmap("/home/Robinix/res/img/mouse/mouse_check.bmp");

  //If one of the mouse pointer bmps was not correctly allocated, destroy the level and return NULL
  if(l_ptr->mouse_bmps[M_OVER_NOTHING] == NULL || l_ptr->mouse_bmps[M_OVER_DOOR] == NULL) {
    destroy_level(&l_ptr);
    return NULL;
  }

  //Setting mouse_over initial state
  l_ptr->current_mouse_over = M_OVER_NOTHING;

  return l_ptr;
}

//Helper function for clearing the array of guards
static void clear_guard_arr(Level * l_ptr) {
  //If there are guards stored, destroy them
  if(l_ptr->n_guards > 0) {
    int i;
    for(i = 0; i < l_ptr->n_guards; i++){
      //Destroying a guard if it is allocated
      destroy_guard(&(l_ptr->guards[i]));
    }

    //Updating the number of guards stored variable
    l_ptr->n_guards = 0;

    //Deallocating space in memory for the array (** was used so this must be done)
    free(l_ptr->guards);
    l_ptr->guards = NULL;
  }
}

//Helper function for clearing the array of doors
static void clear_door_arr(Level * l_ptr) {
  //If there are doors stored, destroy them
  if(l_ptr->n_doors > 0) {
    int i;
    for(i = 0; i < l_ptr->n_doors; i++){
      //Destroying a doors if it is allocated
      destroy_door(&(l_ptr->doors[i]));
    }

    //Updating the number of doors stored variable
    l_ptr->n_doors = 0;

    //Deallocating space in memory for the array (** was used so this must be done)
    free(l_ptr->doors);
    l_ptr->doors = NULL;
  }
}

//Helper function for clearing the array of coins
static void clear_coin_arr(Level * l_ptr) {
  //If there are coins stored, destroy them
  if(l_ptr->n_coins > 0) {
    int i;
    for(i = 0; i < l_ptr->n_coins; i++){
      //Destroying a coin if it is allocated
      destroy_coin(&(l_ptr->coins[i]));
    }

    //Updating the number of coins stored variable
    l_ptr->n_coins = 0;

    //Deallocating space in memory for the array (** was used so this must be done)
    free(l_ptr->coins);
    l_ptr->coins = NULL;
  }
}

void destroy_level(Level ** l_ptr) {
  //If the pointer is already pointing to NULL, we do nothing
  if(*l_ptr == NULL) {
    return;
  }

  //Deallocating the player
  destroy_player(&((*l_ptr)->player));

  //Clearing the guard array (Calls destroy_guard for each event and deallocates the array itself)
  clear_guard_arr(*l_ptr);

  //Deallocating the treasure
  destroy_treasure(&((*l_ptr)->treasure));

  //Clearing the coin array
  clear_coin_arr(*l_ptr);

  //CLearing the doors array
  clear_door_arr(*l_ptr);

  //Deallocating the exit
  destroy_exit(&((*l_ptr)->exit));

  //Clearing the background bitmap stored
  deleteBitmap((*l_ptr)->background_bmp);
  //Clearing the level walls bmp stored
  deleteBitmap((*l_ptr)->level_walls);

  //Clearing the mouse bitmaps stored
  deleteBitmap((*l_ptr)->mouse_bmps[M_OVER_NOTHING]);
  deleteBitmap((*l_ptr)->mouse_bmps[M_OVER_DOOR]);

  //NOTE: Don't forget to update with more deallocations if there are any, eventually

  //Finally, deallocating Level object
  free(*l_ptr);
  //Setting the pointer to the Level struct to NULL so we can know that the object has been deallocated
  //(This is the reason for using a Level ** and not a simple Level * like in all the other functions)
  *l_ptr = NULL;
}

////Creating and destroying of helper objects

Treasure * create_treasure(long startx, long starty) {
  Treasure * t_ptr = malloc(sizeof *t_ptr);

  if(t_ptr == NULL) {
    return NULL;
  }

  if(is_christmas_time()) {
    t_ptr->bmp = loadBitmap("/home/Robinix/res/img/levels/xmas_treasure.bmp");
  } else {
    t_ptr->bmp = loadBitmap("/home/Robinix/res/img/levels/closed_treasure.bmp");
  }

  if(t_ptr->bmp == NULL) {
    destroy_treasure(&t_ptr);
    return NULL;
  }

  //Setting starting values
  t_ptr->picked_up = false;
  t_ptr->x = startx;
  t_ptr->y = starty;

  return t_ptr;
}

void draw_treasure(Treasure * t_ptr) {
  if(t_ptr == NULL) {
    return;
  }

  if(!t_ptr->picked_up) {
    drawBitmap(t_ptr->bmp, t_ptr->x, t_ptr->y);
  }
}

void destroy_treasure(Treasure ** t_ptr) {
  if(*t_ptr == NULL) {
    return;
  }

  deleteBitmap((*t_ptr)->bmp);

  free(*t_ptr);
  *t_ptr = NULL;
}

Door * create_door(long startx, long starty, bool closed_at_start) {
  Door * d_ptr = malloc(sizeof *d_ptr);

  if (d_ptr == NULL) {
    return NULL;
  }

  d_ptr->closed_bmp = loadBitmap("/home/Robinix/res/img/levels/closed_door.bmp");

  if(d_ptr->closed_bmp == NULL) {
    destroy_door(&d_ptr);
    return NULL;
  }

  d_ptr->open_bmp = loadBitmap("/home/Robinix/res/img/levels/open_door.bmp");

  if(d_ptr->open_bmp == NULL) {
    destroy_door(&d_ptr);
    return NULL;
  }

  //Setting starting values
  d_ptr->x = startx;
  d_ptr->y = starty;
  d_ptr->closed = closed_at_start;
  d_ptr->hovered = false;

  return d_ptr;
}

Bitmap * door_get_current_bitmap(Door * d_ptr) {
  if(d_ptr == NULL) {
    return NULL;
  }

  if(d_ptr->closed) {
    return d_ptr->closed_bmp;
  } else {
    return d_ptr->open_bmp;
  }
}

bool is_mouse_over_door(Door * d_ptr, Bitmap * mouse_bmp, long mouseX, long mouseY) {
  //AABB collision is enough for this
  //Was using AABB considering mouse as a point but now considering the full area of the mouse for better accuracy
  return (d_ptr->x < mouseX + mouse_bmp->bitmapInfoHeader.width && d_ptr->x + door_get_current_bitmap(d_ptr)->bitmapInfoHeader.width > mouseX)
    && (mouseY < d_ptr->y + door_get_current_bitmap(d_ptr)->bitmapInfoHeader.height && mouse_bmp->bitmapInfoHeader.height + mouseY > d_ptr->y);
}

void draw_door(Door * d_ptr) {
  if(d_ptr == NULL) {
    return;
  }

  if(d_ptr->closed) {
    drawBitmap(d_ptr->closed_bmp, d_ptr->x, d_ptr->y);
  } else {
    drawBitmap(d_ptr->open_bmp, d_ptr->x, d_ptr->y);
  }
}

void destroy_door(Door ** d_ptr) {

  if(*d_ptr == NULL) {
    return;
  }

  deleteBitmap((*d_ptr)->closed_bmp);
  deleteBitmap((*d_ptr)->open_bmp);

  free(*d_ptr);
  *d_ptr = NULL;
}

Coin * create_coin(long startx, long starty) {
  Coin * c_ptr = malloc(sizeof *c_ptr);

  if(c_ptr == NULL) {
    return NULL;
  }

  c_ptr->bmp = loadBitmap("/home/Robinix/res/img/levels/golden_coin.bmp");

  if(c_ptr->bmp == NULL) {
    destroy_coin(&c_ptr);
    return NULL;
  }

  //Setting starting values
  c_ptr->picked_up = false;
  c_ptr->x = startx;
  c_ptr->y = starty;

  return c_ptr;
}

void draw_coin(Coin * c_ptr) {
  if(c_ptr == NULL) {
    return;
  }

  if(!c_ptr->picked_up) {
    drawBitmap(c_ptr->bmp, c_ptr->x, c_ptr->y);
  }
}

void destroy_coin(Coin ** c_ptr) {
  if(*c_ptr == NULL) {
    return;
  }

  deleteBitmap((*c_ptr)->bmp);

  free(*c_ptr);
  *c_ptr = NULL;
}

Exit * create_exit(long startx, long starty, bool superlocked) {
  Exit * ex_ptr = malloc(sizeof *ex_ptr);

  if(ex_ptr == NULL) {
    return NULL;
  }

  //Loading closed bitmap
  ex_ptr->closed_sprite = loadBitmap("/home/Robinix/res/img/levels/exit_closed.bmp");

  if(ex_ptr->closed_sprite == NULL) {
    destroy_exit(&ex_ptr);
    return NULL;
  }

  //Loading superlocked bitmap
  ex_ptr->superlocked_sprite = loadBitmap("/home/Robinix/res/img/levels/exit_superlocked.bmp");

  if(ex_ptr->superlocked_sprite == NULL) {
    destroy_exit(&ex_ptr);
    return NULL;
  }

  //Loading open sprite (animated!)
  char * open_sprite_paths[] = {"/home/Robinix/res/img/levels/exit_open1.bmp", "/home/Robinix/res/img/levels/exit_open2.bmp", "/home/Robinix/res/img/levels/exit_open3.bmp", "/home/Robinix/res/img/levels/exit_open4.bmp"};
  //Arguments are paths, n_paths and frames per bitmap
  ex_ptr->open_sprite = create_sprite(open_sprite_paths, 4, 15);

  if(ex_ptr->open_sprite == NULL) {
    destroy_exit(&ex_ptr);
    return NULL;
  }

  //Setting starting values
  ex_ptr->x = startx;
  ex_ptr->y = starty;
  ex_ptr->exit_state = (superlocked ? EXIT_SUPERLOCKED : EXIT_CLOSED);

  return ex_ptr;
}

void draw_exit(Exit * ex_ptr) {
  if(ex_ptr == NULL) {
    return;
  }

  switch(ex_ptr->exit_state) {
    case EXIT_SUPERLOCKED:
      drawBitmap(ex_ptr->superlocked_sprite, ex_ptr->x, ex_ptr->y);
      break;
    case EXIT_CLOSED:
      drawBitmap(ex_ptr->closed_sprite, ex_ptr->x, ex_ptr->y);
      break;
    case EXIT_OPEN:
      draw_sprite(ex_ptr->open_sprite, ex_ptr->x, ex_ptr->y);
      break;
  }
}

void exit_goto_next_state(Exit * ex_ptr) {
  if(ex_ptr != NULL) {
    if(ex_ptr->exit_state == EXIT_SUPERLOCKED) {
      ex_ptr->exit_state = EXIT_CLOSED;
    } else if(ex_ptr->exit_state == EXIT_CLOSED) {
      ex_ptr->exit_state = EXIT_OPEN;
    }
  }
}

void destroy_exit(Exit ** ex_ptr) {
  if(*ex_ptr == NULL) {
    return;
  }

  deleteBitmap((*ex_ptr)->closed_sprite);
  deleteBitmap((*ex_ptr)->superlocked_sprite);
  destroy_sprite(&((*ex_ptr)->open_sprite));

  free(*ex_ptr);
  *ex_ptr = NULL;
}

////Drawing
static void level_draw_background(Level * l_ptr) {
  drawFullscreenBitmap(l_ptr->background_bmp);
}

static void level_draw_walls(Level * l_ptr) {
  drawBitmap(l_ptr->level_walls, 0, 0);
}

static void level_draw_treasure(Level * l_ptr) {
  draw_treasure(l_ptr->treasure);
}

static void level_draw_coins(Level * l_ptr) {
  int i;
  for(i = 0; i < l_ptr->n_coins; i++) {
    draw_coin(l_ptr->coins[i]);
  }
}

static void level_draw_exit(Level * l_ptr) {
  draw_exit(l_ptr->exit);
}

static void level_draw_doors(Level * l_ptr) {
  int i;
  for(i = 0; i < l_ptr->n_doors; i++) {
    draw_door(l_ptr->doors[i]);
  }
}

static void level_draw_guards(Level * l_ptr) {
  int i;
  for(i = 0; i < l_ptr->n_guards; i++) {
    draw_guard(l_ptr->guards[i]);
  }
}

static void level_draw_player(Level * l_ptr) {
  if(l_ptr->player != NULL) {
    draw_player(l_ptr->player);
  }
}

static void level_draw_mouse(Level * l_ptr, long mouseX, long mouseY) {
  drawBitmap(l_ptr->mouse_bmps[l_ptr->current_mouse_over], mouseX, mouseY);
}

void draw_level(Level * l_ptr, long mouseX, long mouseY) {
  if(l_ptr == NULL) {
    return;
  }

  level_draw_background(l_ptr);
  level_draw_walls(l_ptr);
  level_draw_treasure(l_ptr);
  level_draw_coins(l_ptr);
  level_draw_exit(l_ptr);
  level_draw_doors(l_ptr);
  level_draw_guards(l_ptr);
  level_draw_player(l_ptr);
  level_draw_mouse(l_ptr, mouseX, mouseY);
}

////Updating
static void level_update_player(Level * l_ptr, Robinix * rob) {
  //Do nothing if there is no player allocated
  if(l_ptr->player == NULL) {
    return;
  }

  //Player speed update based on pressed keys
  if(rob->currstate.w_pressed) {
    set_player_moving_up(l_ptr->player);
  } else if(rob->currstate.s_pressed) {
    set_player_moving_down(l_ptr->player);
  } else {
    set_player_stopped_y(l_ptr->player);
  }

  if(rob->currstate.a_pressed) {
    set_player_moving_left(l_ptr->player);
  } else if(rob->currstate.d_pressed) {
    set_player_moving_right(l_ptr->player);
  } else {
    set_player_stopped_x(l_ptr->player);
  }

  //The angle from the player to the mouse is calulated considering the vector from the player to the mouse
  int player_center_x = l_ptr->player->x + get_player_current_bitmap(l_ptr->player)->bitmapInfoHeader.width/2;
  int player_center_y = l_ptr->player->y + get_player_current_bitmap(l_ptr->player)->bitmapInfoHeader.height/2;
  int mouse_center_x = rob->currstate.mouseX + l_ptr->mouse_bmps[l_ptr->current_mouse_over]->bitmapInfoHeader.width/2;
  int mouse_center_y = rob->currstate.mouseY + l_ptr->mouse_bmps[l_ptr->current_mouse_over]->bitmapInfoHeader.height/2;

  //dx and dy are the coordinates of said vector
  int dx = player_center_x - mouse_center_x;
  int dy = player_center_y - mouse_center_y;

  //To get the angle relative to the 1,0 vector it is enough to use atan2, which was defined precisely for this usage
  //We have to pass in -dy because in the considered mathematical y axis, y increases when going up
  //However, in this implementation, y increases when going down. Therefore we invert the sign
  double angle = atan2(-dy, dx);

  //Setting the angle in the player object
  set_player_angle(l_ptr->player, angle);

  //Collisions with walls

  //NOTE: Using player's idle BMP as hitbox. It is not perfect since we aren't considering rotation or run animation but at least it won't get the player stuck
  //TODO: Give the player a constant hitbox that considers the rotation and running animations and use that instead
  if(l_ptr->level_walls != NULL) {
    //Before updating the player with its speed checking for wall collisions to see if it is necessary to stop the player in any axis
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y, l_ptr->level_walls, 0, 0)) {
      //Is going to collide in the x axis, stop x movement
      set_player_stopped_x(l_ptr->player);
    }
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x, l_ptr->player->y + l_ptr->player->speedY, l_ptr->level_walls, 0, 0)) {
      //Is going to collide in the y axis, stop y movement
      set_player_stopped_y(l_ptr->player);
    }
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y + l_ptr->player->speedY, l_ptr->level_walls, 0, 0)) {
      //Is going to collide in the x and y axis, stop xy movement
      set_player_stopped_x(l_ptr->player);
      set_player_stopped_y(l_ptr->player);
      //After stopping in both axis, no further "stopping" is possible, so we just return to be more efficient (and player position would not change anyway)
      return;
    }
  }

  //Collisions with doors (so that player doesn't noclip through them)

  //NOTE && TODO: Same as above
  if(l_ptr->n_doors > 0) {
    int i;
    for(i = 0; i < l_ptr->n_doors; i++) {
      //Before updating the player with its speed checking for door collisions to see if it is necessary to stop the player in any axis
      if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y, door_get_current_bitmap(l_ptr->doors[i]), l_ptr->doors[i]->x, l_ptr->doors[i]->y)) {
        //Is going to collide in the x axis, stop x movement
        set_player_stopped_x(l_ptr->player);
      }
      if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x, l_ptr->player->y + l_ptr->player->speedY, door_get_current_bitmap(l_ptr->doors[i]), l_ptr->doors[i]->x, l_ptr->doors[i]->y)) {
        //Is going to collide in the y axis, stop y movement
        set_player_stopped_y(l_ptr->player);
      }
      if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y + l_ptr->player->speedY, door_get_current_bitmap(l_ptr->doors[i]), l_ptr->doors[i]->x, l_ptr->doors[i]->y)) {
        //Is going to collide in the x and y axis, stop xy movement
        set_player_stopped_x(l_ptr->player);
        set_player_stopped_y(l_ptr->player);
        //After stopping in both axis, no further "stopping" is possible, so we just return to be more efficient (and player position would not change anyway)
        return;
      }
    }
  }

  //Collisions with level border (to prevent player from leaving screen)

  //NOTE && TODO: Same as above
  if(l_ptr->level_border != NULL) {
    //Before updating the player with its speed checking for level border collisions to see if it is necessary to stop the player in any axis
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y, l_ptr->level_border, 0, 0)) {
      //Is going to collide in the x axis, stop x movement
      set_player_stopped_x(l_ptr->player);
    }
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x, l_ptr->player->y + l_ptr->player->speedY, l_ptr->level_border, 0, 0)) {
      //Is going to collide in the y axis, stop y movement
      set_player_stopped_y(l_ptr->player);
    }
    if(check_if_bitmaps_collided(l_ptr->player->playerSprite->bmps[0], l_ptr->player->x + l_ptr->player->speedX, l_ptr->player->y + l_ptr->player->speedY, l_ptr->level_border, 0, 0)) {
      //Is going to collide in the x and y axis, stop xy movement
      set_player_stopped_x(l_ptr->player);
      set_player_stopped_y(l_ptr->player);
      //After stopping in both axis, no further "stopping" is possible, so we just return to be more efficient (and player position would not change anyway)
      return;
    }
  }

  //Player position update
  update_player(l_ptr->player);
}

static void level_update_guards(Level * l_ptr){
  int i;
  for (i = 0; i < l_ptr->n_guards; i++) {
    update_guard(l_ptr->guards[i]);
  }
}

static void level_test_collisions(Level * l_ptr, Robinix * rob) {

  if(l_ptr->player == NULL) {
    return;
  }
  //Checking collisions from player with guards (adds event to buffer in case they collide)
  int i;
  for(i = 0; i < l_ptr->n_guards; i++) {
    if(check_if_bitmaps_collided_rotated_w_non_rotated(get_player_current_bitmap(l_ptr->player), l_ptr->player->x, l_ptr->player->y, l_ptr->player->angle,
	   get_guard_current_bitmap(l_ptr->guards[i]), l_ptr->guards[i]->guardX, l_ptr->guards[i]->guardY)) {
       add_event_to_buffer(rob, create_event(PLAYER_COLLIDE_WITH_GUARD, 0, 0, '?', NULL));
      //It should only be possible for one collision to happen at a time (and anyways, one is enough to result in a loss)
      break;
    }
  }
  //Checking player collisions with treasure
  if(l_ptr->treasure != NULL && !l_ptr->treasure->picked_up) {
    if(check_if_bitmaps_collided_rotated_w_non_rotated(get_player_current_bitmap(l_ptr->player), l_ptr->player->x, l_ptr->player->y, l_ptr->player->angle,
    l_ptr->treasure->bmp, l_ptr->treasure->x, l_ptr->treasure->y)) {
      l_ptr->treasure->picked_up = true;
      //Setting exit to the next state on treasure pick up
      exit_goto_next_state(l_ptr->exit);
      //Sending event about getting treasure due to multiplayer
      add_event_to_buffer(rob, create_event(PLAYER_GOT_TREASURE, 0, 0, '?', NULL));
    }
  }
  //Checking player collisions with coins
  //i defined above, used for guard loop
  for(i = 0; i < l_ptr->n_coins; i++) {
    if(!l_ptr->coins[i]->picked_up) {
      //If coin has not yet been picked up, test for collision with player
      if(check_if_bitmaps_collided_rotated_w_non_rotated(get_player_current_bitmap(l_ptr->player), l_ptr->player->x, l_ptr->player->y, l_ptr->player->angle,
                                                         l_ptr->coins[i]->bmp, l_ptr->coins[i]->x, l_ptr->coins[i]->y)) {
        l_ptr->coins[i]->picked_up = true;
        add_event_to_buffer(rob, create_event(PLAYER_GOT_COIN, 0, 0, '?', NULL));
      }
    }
  }
  //Checking player collisions with exit (Can only collide with exit if exit is open - aka not closed)
  if(l_ptr->exit != NULL && l_ptr->exit->exit_state == EXIT_OPEN) {
    if(check_if_bitmaps_collided_rotated_w_non_rotated(get_player_current_bitmap(l_ptr->player), l_ptr->player->x, l_ptr->player->y, l_ptr->player->angle,
       l_ptr->exit->closed_sprite, l_ptr->exit->x, l_ptr->exit->y)) {
      add_event_to_buffer(rob, create_event(PLAYER_COLLIDE_WITH_EXIT, 0, 0, '?', NULL));
    }
  }
}


//NOTE: rob pointer is necessary due to sending events
void update_level(Level * l_ptr, Robinix * rob) {
  //Update player
  level_update_player(l_ptr, rob);
  //Update guards
  level_update_guards(l_ptr);
  //Ensuring mouse does not go offscreen (kind of ugly but I don't see any good way to make it better besides needless pixel perfect collision)
  limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), l_ptr->mouse_bmps[l_ptr->current_mouse_over]->bitmapInfoHeader.width, l_ptr->mouse_bmps[l_ptr->current_mouse_over]->bitmapInfoHeader.height);
  //Test for guard, coin, treasure and exit collisions
  level_test_collisions(l_ptr, rob);
}

////Updating based on mouse
void level_update_mouse_over(Level * l_ptr, long mouseX, long mouseY) {
  if(l_ptr == NULL) {
    return;
  }

  ///Checking if the mouse is over each door

  //Starting at OVER_NOTHING, if it is over one door it will immediately be OVER_DOOR (we are not changing back)
  l_ptr->current_mouse_over = M_OVER_NOTHING;

  int i;
  for(i = 0; i < l_ptr->n_doors; i++) {
    if(is_mouse_over_door(l_ptr->doors[i], l_ptr->mouse_bmps[l_ptr->current_mouse_over], mouseX, mouseY)) {
      l_ptr->doors[i]->hovered = true;
      l_ptr->current_mouse_over = M_OVER_DOOR;
    } else {
      l_ptr->doors[i]->hovered = false;
    }
  }
}

void level_handle_mouse_click(Level * l_ptr) {
  if(l_ptr == NULL) {
    return;
  }

  if(l_ptr->current_mouse_over == M_OVER_NOTHING) {
    //If mouse is currently over nothing then it is surely not over a door
    return;
  }

  //If a door is hovered when the mouse is clicked then its closed state will be inverted
  int i;
  for(i = 0; i < l_ptr->n_doors; i++) {
    if(l_ptr->doors[i]->hovered) {
      l_ptr->doors[i]->closed = !(l_ptr->doors[i]->closed);
    }
  }
}

void level_remote_got_treasure(Level * l_ptr) {
  if(l_ptr == NULL) {
    return;
  }

  exit_goto_next_state(l_ptr->exit);
}
