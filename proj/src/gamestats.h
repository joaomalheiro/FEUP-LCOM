#ifndef _GAMESTATS_H
#define _GAMESTATS_H

#include "rtc.h"

/** @defgroup robinix robinix
 * @{
 *
 * Functions and structs for storing and operating over Game Statistics (number of coins picked up and time elapsed) and allow for, for example, calculating player score at the end of the game
 */

typedef struct {
  unsigned int n_coins_picked_up;
  Date_obj time_elapsed;
} GameStats;

/**
 * @brief Creates a new and reset GameStats object
 * @return Returns a pointer to a valid GameStats object or NULL in case of failure
 */
GameStats * create_gamestats();

/**
 * @brief Destroys the passed in GameStats object, correctly freeing memory and setting the passed pointer to NULL
 * @param gs_ptr GameStats object to destroy
 */
void destroy_gamestats(GameStats ** gs_ptr);

/**
 * @brief Draws the relevant contents of the GameStats object on screen, to be used while playing
 * @param gs_ptr GameStats object to draw
 */
void gamestats_draw(GameStats * gs_ptr);

/**
 * @brief Increments the coin counter of the GameStats object
 * @param gs_ptr GameStats object to use
 */
void gamestats_tick_coins(GameStats * gs_ptr);

/**
 * @brief Counts up the time of the GameStats object
 * @param gs_ptr GameStats object to use
 */
void gamestats_tick_time(GameStats * gs_ptr);

/**
 * @brief Calculates player score based on player stats
 * @param  gs_ptr GameStats object to calculate the score from
 * @return        Returns the score of the player
 */
unsigned long gamestats_calculate_score(GameStats * gs_ptr);

/**
 * @brief Gets the time taken by the player
 * @param  gs_ptr GameStats object to use
 * @return        String in HH:MM:SS format representative of time taken by the player, or NULL in case of error
 */
char * gamestats_get_time_taken(GameStats * gs_ptr);

#endif /* __GAMESTATS_H */
