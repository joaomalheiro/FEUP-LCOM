#ifndef __TEST4_H
#define __TEST4_H

////Typedefs:
//State enum
typedef enum {
	INIT = 0,
	DRAWING,
	COMPLETED
} state_enum;

//Event enum
typedef enum {
	RB_DOWN = 0,
	RB_UP,
	MOVE
} event_enum;

//The definition of a state machine struct
typedef struct {
	short objective;
	short progress;
	state_enum state;
} state_st;

//The definition of an event
typedef struct {
	signed long x;
	signed long y;
	event_enum event;
} event_st;

////Function prototypes

/** @defgroup test4 test4
 * @{
 *
 * Functions for testing the kbd code
 */

/**
 * @brief To test packet reception via interrupts
 *
 * Displays the packets received from the mouse
 * Exits after receiving the number of packets specified in argument
 *
 * @param cnt Number of packets to receive and display before exiting
 *
 * @return Return 0 upon success and non-zero otherwise
 */

int mouse_test_packet(unsigned short cnt);



/**
 * @brief To test handling of more than one interrupt
 *
 *  Similar test_packet() except that it
 *  should terminate if no packets are received for idle_time seconds
 *
 * @param idle_time Number of seconds without packets before exiting
 *
 * @return Return 0 upon success and non-zero otherwise
 */

int mouse_test_async(unsigned short idle_time);


/**
 * @brief To test PS/2 remote mode operation
 *
 * Configures the PS/2 to operate in remote mode
 * Periodically requests a packet from the mouse,
 * and displays the packets received from the mouse
 * Exits after receiving the number of packets specified in argument
 *
 * @param period Period in milliseconds
 * @param cnt Number of packets to receive and display before exiting
 *
 * @return Return 0 upon success and non-zero otherwise
 */

int mouse_test_remote(unsigned long period, unsigned short cnt);

/**
 * @brief To test state machine implementation
 *
 *  Similar mouse_test_packet() except that it
 *  should terminate if user moves the mouse continuously
 *    with a positive slope
 *
 * @param length minimum length of movement (mouse units)
 *  in the x-direction. If positive, upwards movement, otherwise
 *  downwards.
 *
 *
 * @return Return 0 upon success and non-zero otherwise
 */


int mouse_test_gesture(short length);

/**
 * @brief Updates the current state based on received event
 * @param evt The event that ocurred
 */
void mouse_update_state(event_st * evt);

/**
 * @brief Detects event ocurrences and calls state updating function
 */
void mouse_update_events();
#endif /* __TEST_4.H */
