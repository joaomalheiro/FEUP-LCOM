#include <minix/syslib.h>
#include <minix/drivers.h>
/* for tickdelay and micros_to_ticks */
#include <minix/sysutil.h>
#include <stdbool.h>
#include "keyboard.h"
#include "i8042.h"
#include "robinix.h"
#include "game.h"

//The bit that differentiates a make from a break code
#define BREAK_BIT BIT(7)
//Generates the corresponding make code from a scan code
#define GET_MAKECODE(n) ((n) & 0x7f)

/////Useful private variables
//Hook ID for subscribing to keyboard interrupts
static int kb_hookID = 1;
//Currently read scancode
static unsigned char scancode;

//Flag that indicates if a scancode is currently 2 bytes or not
static bool is2byte = false;

///NOTE: for special keys not representable in a single character:
//Escape: !
//ENTER : E
//Caps Lock: C
//BACKSPACE: B
//...

//Now using a direct map, in which a make code corresponds directly to the associated character
//(? is bound to keys that do not matter / are not being used)
//Split by keyboard lines, more or less (First line in the array is a dummy index because arrays start at 0 and not 1 but makecodes start at 1)
//First line starts at Escape and ends at ' « BACKSPACE
//Second line starts at TAB and ends at ENTER (left CTRL follows because it is there in the list, for some reason)
//Third line starts at 'a' and ends at ç º \ right shift ~
//Foruth line starts at 'z' and ends at , . - left shift numpad* left alt
static const unsigned char scancode_to_key_map[] = {
	'?',
	'!', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '?', '?', 'B',
	'?', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '?', '?', 'E', '?',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'c', '?', '?', '?', '?',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', '?', '.', '?', '?', '?', '?',
	' '
};

#define MAX_SCANCODE 0x39 /* Last makecode to consider, after this one disregard the scancodes (to prevent from having to define a gigantic array) */

int keyboard_subscribe_int() {

	//Variable used for preserving kb_hookID value that will later be used
	int temp = kb_hookID;

	if(sys_irqsetpolicy(KBC_IRQ,IRQ_REENABLE | IRQ_EXCLUSIVE, &kb_hookID) != OK) {
		printf("keyboard_subscribe_int::Error setting IRQ policy!\n");
		return -1;
	}

	if(sys_irqenable(&kb_hookID)!= OK){
		printf("keyboard_subscribe_int::Error enabling interrupts on the IRQ line!\n");
		return -2;
	}

	//Everything went as expected, returning bitmask of the kb_hookID for interrupt handling
	return BIT(temp);
}

int keyboard_unsubscribe_int() {

	if(sys_irqdisable(&kb_hookID) != OK){
		printf("keyboard_unsubscribe_int::Error disabling interrupts on the IRQ line\n");
		return 1;
	}

	if(sys_irqrmpolicy(&kb_hookID) != OK){
		printf("keyboard_unsubscribe_int::Error unsubscribing the previous subscription of the interrupt notification on the IRQ line!\n");
		return 2;
	}

	//Everything went as expected
	return 0;
}

int keyboard_IH() {
	//Reading keyboard IH
	int read_errorlevel = keyboard_read_scancode_IH();

	//Error level of 1 signifies no data was read, and we should just continue (without sending events or processing inputs)
	//Basically if 1 is returned the variable "scancode" is not updated
	if(read_errorlevel == 1) {
		return 0;
	}

	if(read_errorlevel != 0){
		return -1;
	}

	//Scancode is now stored in 'scancode' global variable

	if(scancode == TWO_BYTE_SCANCODE) {
		is2byte = true;
		return 0;
	}

	//We are currently not processing 2 byte scancodes so we just skip until the next interrupt by returning 0
	if(is2byte) {
		is2byte = false;
		return 0;
	}

	//Getting the pointer to the game object
	Robinix * rob = get_rob();

	//Sending event to game object
	if(rob != NULL) {
		if(GET_MAKECODE(scancode) > MAX_SCANCODE) {
			//Not in the scancode_to_key_map map, return 0 (no errors)
			return 0;
		}

		event_enum type;

		if(scancode & BREAK_BIT) {
			//If the break bit is activated, it is a break code, KEY_UP event
			type = KEY_UP;
		} else {
			//Otherwise, is key down event
			type = KEY_DOWN;
		}

		//Getting the pressed key from the scancode_to_key_map map
		char pressed_key = scancode_to_key_map[GET_MAKECODE(scancode)];

		if(pressed_key == '?') {
			//Scancode does not correspond to any tracked key so we return without sending any event
			return 0;
		}

		//Creating the event
		Event * kb_evt = create_event(type, 0, 0, pressed_key, NULL);

		//Sending the event (No need to check for NULL on creation since the event is not added if it is NULL)
		add_event_to_buffer(rob, kb_evt);
	}

	//Regardless of adding event to buffer or not, everything went as expected
	return 0;
}

int keyboard_read_scancode_IH(){
  unsigned int nretries;
	/*unsigned long statreg;
	unsigned long readOutbuf;*/

  //Due to the potential delay of the KBC, the read operation is attempted several times with a small delay
	for(nretries = 0; nretries <= KBC_MAX_RETRIES; ++nretries) {

		/*if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
			//Error reading status register
			continue;
		}

		//If the i8042 output buffer is not empty (output buffer full, OBF flag is 1)
		if(statreg & KBC_OBF) {
			//If the data is mouse input, we return 1 without attempting to read (no error but do not want to read too)
			//1 signifies no data was read so no events should be sent
			if(statreg & KBC_AUX) {
				return 1;
			}
			//If the status byte indicates that the data is valid, we will attempt to read the data
			if((statreg & (KBC_PARITY | KBC_TIMEOUT)) == 0){
				if(sys_inb(KBC_OUT_BUF, &readOutbuf) != OK){
					printf("keyboard_read_scancode::Error reading output buffer\n");
					return -1;
				} else{
					//Storing the data
					scancode = (unsigned char) readOutbuf;
					//Everything went as expected
					return 0;
				}
			} else {
				printf("keyboard_read_scancode::Invalid data in OUT_BUF, parity or timeout error\n");
				return -2;
			}
		}
		*/

		//Invoking the Assembly IH
		asm_readscancode();

		//The global variables are now altered

		if(asm_errorlevel != 0){
			//There was an error, returning the errorlevel (that is different than 0)
			return asm_errorlevel;
		}

		//Otherwise, there were no errors, returning 0 and passing the read scancode using the passed in variable
		scancode = asm_scancode;
		return 0;

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
	}

  //If no data is read after the maximum number of retries
	return 0;
}

int keyboard_write_kbccommand(unsigned char kbccmd){
  unsigned long status;
  unsigned int nretries;

  for(nretries = 0; nretries < KBC_MAX_RETRIES; ++nretries){

    if(sys_inb(KBC_STATUS_REG, &status) != OK){
      //Error reading status register, retrying
      continue;
    }

    //If the i8042 input buffer is empty (IBF flag is 0)
		if((status & KBC_IBF) == 0) {
      //We can write the passed command
      if (sys_outb(KBC_CMD_REG, kbccmd) != OK){
				continue;
			}
      //NOTE: This is a command that has no arguments,
      //Arguments should be passed through the KBC_IN_BUF register

      //Everything went as expected
      return 0;
		}

    //If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
  }

  printf("keyboard_write_kbccommand::The KBC command could not be written after %d tries.\n", KBC_MAX_RETRIES);
	return -1;
}

int keyboard_write_cmdbyte(unsigned char cmdbyte){

  if(keyboard_write_kbccommand(KBC_WRITECMDBYTE) != 0){
    printf("keyboard_write_cmdbyte::Error writing 'Write Command Byte' command to the KBC\n");
    return -1;
  }

  unsigned int nretries;
	unsigned long statreg;

  for(nretries = 0; nretries < KBC_MAX_RETRIES; ++nretries){

    if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
      //Error reading status register, retrying
      continue;
    }

    //If the i8042 input buffer is empty (IBF flag is 0)
		if((statreg & KBC_IBF) == 0) {
      //We can write command byte into the IN_BUF
			if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
	      //Error reading status register, retrying
	      continue;
	    }

      //Everything went as expected
      return 0;
		}

    //If this is not the case we just continue to another iteration after a small delay, retrying the read operation
	  tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
  }

  printf("keyboard_write_cmdbyte::The keyboard command byte could not be written after %d tries.\n", KBC_MAX_RETRIES);
  return -2;
}

int keyboard_read_cmdbyte(unsigned char *cmdbyte){

  if(keyboard_write_kbccommand(KBC_READCMDBYTE) != 0){
    printf("keyboard_read_cmdbyte::Error writing 'Read Command Byte' command to the KBC\n");
    return -1;
  }

  unsigned int nretries;
	unsigned long statreg;
	unsigned long readOutbuf;

	for(nretries = 0; nretries <= KBC_MAX_RETRIES; ++nretries) {

		if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
			//Error reading status register
			continue;
		}

		//If the i8042 output buffer is not empty (output buffer full, OBF flag is 1)
		if(statreg & KBC_OBF) {
			//If the status byte indicates that the data is valid, we will attempt to read the data
			if((statreg & (KBC_PARITY | KBC_TIMEOUT)) == 0){
				if(sys_inb(KBC_OUT_BUF, &readOutbuf) != OK){
					printf("keyboard_read_cmdbyte::Error reading output buffer\n");
					return -1;
				} else{
					//Storing the data
					*cmdbyte = (unsigned char) readOutbuf;
					//Everything went as expected
					return 0;
				}
			} else {
				printf("keyboard_read_cmdbyte::Invalid data in OUT_BUF, parity or timeout error\n");
				return -2;
			}
		}

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
	}

	printf("keyboard_read_cmdbyte::No data was read after %d tries.\n", KBC_MAX_RETRIES);
	return -3;
}

int keyboard_clear_outbuf(){

	unsigned long stat;

	if(sys_inb(KBC_STATUS_REG, &stat)){
		//Error reading status register
		return -1;
	}

	unsigned long trash;

	if(stat & KBC_OBF){
		sys_inb(KBC_OUT_BUF, &trash);
	}

	return 0;
}
