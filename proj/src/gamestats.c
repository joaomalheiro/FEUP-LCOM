#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "gamestats.h"
#include "rtc.h"
#include "rtc_defines.h"
#include "font.h"

GameStats * create_gamestats() {

  //Allocating and checking if allocation was successful
  GameStats * gs_ptr = malloc(sizeof *gs_ptr);

  if (gs_ptr == NULL) {
    return NULL;
  }

  gs_ptr->time_elapsed = (Date_obj){.year = 0, .month=0, .day=0, .hour = 0, .minute = 0, .second = 0};
  gs_ptr->n_coins_picked_up = 0;

  //Returning a pointer to the created object
  return gs_ptr;
}

void destroy_gamestats(GameStats ** gs_ptr) {
  if(*gs_ptr == NULL) {
    return;
  }

  free(*gs_ptr);
  *gs_ptr = NULL;
}

void gamestats_tick_coins(GameStats * gs_ptr) {
  if(gs_ptr == NULL) {
    return;
  }

  gs_ptr->n_coins_picked_up++;
}

void gamestats_tick_time(GameStats * gs_ptr) {
  if(gs_ptr == NULL) {
    return;
  }

  tick_second(&(gs_ptr->time_elapsed));
}

unsigned long gamestats_calculate_score(GameStats * gs_ptr) {
  if(gs_ptr == NULL) {
    return 0;
  }

  if ((gs_ptr->time_elapsed.day != 0) ||(gs_ptr->time_elapsed.month != 0) || (gs_ptr->time_elapsed.year != 0)) {
    return 0;
  }

  return 100 + (gs_ptr->n_coins_picked_up)* 50 + (5000 / get_seconds(&(gs_ptr->time_elapsed)));
}

void gamestats_draw (GameStats *gs_ptr) {
  if(gs_ptr == NULL) {
    return;
  }

  char * date = date_to_time_string(&(gs_ptr->time_elapsed));
  //The maximum value of a 32bit unsigned int is around 4 million - 10 characters (+1 for \0)
  //Add to that the size of "COINS: " (7) and we get 18
  char n_coins[18];
  sprintf(n_coins, "COINS: %u", gs_ptr->n_coins_picked_up);

  string_to_screen(date, "monofonto-22", 10, 10);
  string_to_screen(n_coins, "monofonto-22", 500, 10);

  //Date string now no longer needed (n_coins does not need to be free'd since it was stack allocated)
  free(date);
}

char * gamestats_get_time_taken(GameStats * gs_ptr) {
  if(gs_ptr == NULL) {
    return NULL;
  }

  return date_to_time_string(&(gs_ptr->time_elapsed));
}
