#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/** @defgroup keyboard keyboard
 * @{
 *
 * Main implementation of PS/2 Keyboard Device Driver along with helper functions for KBC communication
 */

/**
 * @brief Assembly interrupt handler to read scan codes
 * @return Return value is irrelevant, the function returns via the "asm_scancode" and "asm_errorlevel" variables
 */
extern unsigned long asm_readscancode();

//Assembly external global variables
extern unsigned char asm_scancode;
extern unsigned char asm_errorlevel;


/*
 * @brief Subscribes keyboard interrupts
 * @return 0 if successful, not 0 if otherwise
 */
int keyboard_subscribe_int();

/**
 * @brief Unsubscribes keyboard interrupts
 * @return 0 if successful, different otherwise
 */
int keyboard_unsubscribe_int();

/**
 * @brief Interrupt handler for the keyboard, sends keyboard events to the game object
 * @return 0 if no problems ocurred, different otherwise
 */
int keyboard_IH();

/**
 * @brief Reads keyboard scancodes for the IH
 * @param  scancode A pointer to where to write the resulting read scancode
 * @return          0 if no problems ocurred, -1 if there was a problem reading the OUT_BUF, -2 if there is invalid data in the OUT_BUF
 */
int keyboard_read_scancode_IH();

/**
 * @brief Writes a command to the KBC, by writing to the register at 0x64.
 *        Arguments must be passed through the IN_BUF (0x60).
 *        Return values are passed through the OUT_BUF (0x60).
 * @param  kbccmd  The command to pass to the KBC
 * @return         0 if successful, different from 0 if otherwise
 */
int keyboard_write_kbccommand(unsigned char kbccmd);

/**
 * @brief Reads the current command byte from the KBC
 * @param  cmdbyte The read command byte
 * @return         0 if no problems ocurred, not 0 if otherwise
 */
int keyboard_read_cmdbyte(unsigned char *cmdbyte);

/**
 * @brief Sends a command byte to the KBC, by writing the 'write command byte' command first and then writing the command byte to the IN_BUF
 * @param  cmdbyte The command byte to write to the KBC
 * @return         0 if no problems ocurred, not 0 otherwise
 */
int keyboard_write_cmdbyte(unsigned char cmdbyte);

/**
 * @brief Clears OUT_BUF
 * @return 0 if no problems ocurred, not 0 otherwise
 */
int keyboard_clear_outbuf();

#endif /* _KEYBOARD_H_ */
