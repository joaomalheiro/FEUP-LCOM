#include <minix/syslib.h>
#include <minix/drivers.h>

/** @defgroup mouse mouse
 * @{
 *
 * Main implementation of PS/2 Mouse Device Driver
 */

extern unsigned long asm_readpacketbyte();

//Assembly external global variables
extern unsigned char asm_readbyte;
extern unsigned char asm_mouse_errorlevel;

/**
 * @brief Subscribes mouse interrupts
 * @return 0 if successful, not 0 if otherwise
 */
int mouse_subscribe_int();

/**
 * @brief Unsubscribes mouse interrupts
 * @return 0 if successful, different otherwise
 */
int mouse_unsubscribe_int();

/**
 * @brief Reads the mouse response to when a command is sent via the KBC
 * @return The response that was read or -1 in case of error
 */
unsigned char mouse_read_response();


/**
 * @brief  Writes a byte to the mouse, forwarded by the KBC
 * @param  mousebyte The byte to pass to the mouse
 * @return           0 if successful, not 0 if otherwise
 */
int mouse_write_byte(unsigned char mousebyte);

/**
 * @brief Mouse interrupt handler, reads packet bytes while handling possible errors, and if it has a full packet, interprets it and sends the respective events to the game object
 * @return Returns 0 if successful, different from 0 if otherwise
 */
int mouse_IH();

/**
 * @brief Receives a packet byte, serving as an IH for mouse interrupts (stream mode)
 * @return 0 if successful or not syncronized (problem dealt with internally), not 0 if there were problems in reading or other operations
 */
int mouse_receive_packet_IH();

/**
 * @brief Reads a byte from the outbuffer, serving as an helper function to the IH
 * @param  readbyte     The byte that will be read, and updated here
 * @param  remote_delay The delay between read attempts
 * @return              0 if successful, not 0 if otherwise/-3 if timed out without reading, not an error when in remote mode
 */
int mouse_read_packetbyte(unsigned char *readbyte, unsigned long remote_delay);

/**
 * @brief Prints the full packet stored in the global variable mouse_packetarr
 */
void mouse_print_packet();
