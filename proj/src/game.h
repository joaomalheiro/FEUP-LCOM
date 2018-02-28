#ifndef __GAME_H
#define __GAME_H

#include "robinix.h"

/** @defgroup game game
 * @{
 *
 * Functions that allow execution of the different parts of the project
 */

/**
 * @brief Returns a pointer to the currently used game object. For use in handlers that send events, for example
 * @return Returns a pointer to the currently used game object
 */
Robinix * get_rob();

/**
 * @brief Plays the game
 * @return Returns 0 if successful, not 0 otherwise
 */
int play_game();

/**
 * @brief Tests Serial Port Transmission
 * @param  string The string to send via serial port
 * @return        0 if successful, not 0 otherwise
 */
int test_uart_tx(char * string);

/**
 * @brief Tests Serial Port Reception
 * @return 0 if successful, not 0 otherwise
 */
int test_uart_rx();

/** @} */


#endif /* __GAME_H */
