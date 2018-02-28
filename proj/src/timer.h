#ifndef __TIMER_H
#define __TIMER_H

/** @defgroup timer timer
 * @{
 *
 * Functions for using the i8254 timers
 */

extern unsigned int timer_interrupt_counter;

/**
 * @brief Changes the operating frequency of a timer
 *
 * Must not change the 3 LSBs (mode and BCD/binary) of the timer's control word.
 *
 * @param timer Timer to configure. (Ranges from 0 to 2)
 * @param freq Timer operating frequency
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_set_frequency(unsigned char timer, unsigned long freq);

/**
 * @brief Subscribes and enables Timer 0 interrupts
 *
 * @return Returns bit order in interrupt mask; negative value on failure
 */
int timer_subscribe_int();

/**
 * @brief Unsubscribes Timer 0 interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_unsubscribe_int();

/**
 * @brief Timer 0 interrupt handler
 *
 * Increments counter or calls relevant functions in game object, if in use
 */
void timer_IH();

/**
 * @brief Reads the input timer configuration via read-back command
 *
 * @param timer Timer whose config to read (Ranges from 0 to 2)
 * @param st    Address of memory position to be filled with the timer config
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_get_conf(unsigned char timer, unsigned char *st);

/**
 * @brief Shows timer configuration
 *
 * Displays in a human friendly way, the configuration of a timer
 *  as read via the read-back command, by providing the values
 *  (and meanings) of the different components of a timer configuration
 *
 * @param conf configuration to display in human friendly way
 * @return Return 0 upon success and non-zero otherwise
 */
int timer_display_conf(unsigned char conf);

#endif /* __TIMER_H */
