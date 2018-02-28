#include "mouse.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdbool.h>
//For tickdelay and micros_to_ticks
#include <minix/sysutil.h>
#include "i8042.h"
//Including the keyboard functions because of reusing code to send commands to the KBC
#include "keyboard.h"
#include "robinix.h"
#include "game.h"

////Private variables
//Hook ID for subscribing to mouse interrupts
static int mouse_hookID = 2;
//Variables for receiving mouse packets
//Array of received bytes, if there are 3 then a full packet was received and can be printed
static unsigned char mouse_packetarr[3];
//Keeps track of which byte of the packet we are at (goes from 0 to 2)
static unsigned int mouse_packetbytecounter = 0;

//For dealing with mouse overflow
//The maximum value for an unsigned byte is used since the sign bit is "external" to the byte
#define MAX_BYTE_VALUE_UNSIGNED 255

////Helper private functions

static bool mouse_is_LB_clicked() {
	return mouse_packetarr[0] & MOUSE_LB;
}

static bool mouse_is_MB_clicked() {
	return mouse_packetarr[0] & MOUSE_MB;
}

static bool mouse_is_RB_clicked() {
	return mouse_packetarr[0] & MOUSE_RB;
}

static int mouse_get_X_movement() {
	//If there was overflow, return the maximum negative or positive value
	if(mouse_packetarr[0] & MOUSE_XOVF) {
		return (mouse_packetarr[0] & MOUSE_XSIGN ? -MAX_BYTE_VALUE_UNSIGNED : MAX_BYTE_VALUE_UNSIGNED);
	}

	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_XSIGN){
		return (-1 << 8) | mouse_packetarr[1];
	} else {
		return (unsigned int) mouse_packetarr[1];
	}
}

static int mouse_get_Y_movement() {
	//If there was overflow, return the maximum negative or positive value
	if(mouse_packetarr[0] & MOUSE_YOVF) {
		return (mouse_packetarr[0] & MOUSE_YSIGN ? -MAX_BYTE_VALUE_UNSIGNED : MAX_BYTE_VALUE_UNSIGNED);
	}

	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_YSIGN){
		return (-1 << 8) | mouse_packetarr[2];
	} else {
		return (unsigned int) mouse_packetarr[2];
	}
}

////Public functions

int mouse_subscribe_int() {

  //Variable used for preserving mouse_hookID value that will later be used
	int temp = mouse_hookID;

	if(sys_irqsetpolicy(MOUSE_IRQ,IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hookID) != OK) {
		printf("mouse_subscribe_int::Error setting IRQ policy!\n");
		return -1;
	}

	if(sys_irqenable(&mouse_hookID)!= OK){
		printf("mouse_subscribe_int::Error enabling interrupts on the IRQ line!\n");
		return -2;
	}

	//Everything went as expected, returning bitmask of the mouse_hookID for interrupt handling
	return BIT(temp);
}

int mouse_unsubscribe_int() {

	if(sys_irqdisable(&mouse_hookID) != OK){
		printf("mouse_unsubscribe_int::Error disabling interrupts on the IRQ line\n");
		return 1;
	}

	if(sys_irqrmpolicy(&mouse_hookID) != OK){
		printf("mouse_unsubscribe_int::Error unsubscribing the previous subscription of the interrupt notification on the IRQ line!\n");
		return 2;
	}

	//Everything went as expected
	return 0;
}

unsigned char mouse_read_response(){

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
					//Error reading the OUT_BUF
					return -1;
				} else {
					//Returning the data
					return (unsigned char) readOutbuf;
				}
			} else {
				//Invalid data in OUT_BUF, parity or timeout error
				return -1;
			}
		}

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
	}

  //The data couldn't be read after the maximum number of retries
  return -1;

}

int mouse_write_byte(unsigned char mousebyte){
	if(keyboard_write_kbccommand(KBC_WRITEBYTETOMOUSE) != 0){
    printf("mouse_write_byte::Error writing 'Write Byte to Mouse' command to the KBC\n");
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
      //We can write mouse byte into the IN_BUF
			if (sys_outb(KBC_IN_BUF, mousebyte) != OK) {
				//Error reading register
				continue;
			}

			//Reading the response to the command
			unsigned char response = mouse_read_response();
			if(response == MOUSE_ACK){
				//Command acknowledged, everything went as expected
	      return 0;
			} else if(response == -1) {
				printf("mouse_write_byte::Error reading mouse response\n");
				return -2;
			} else if(response == MOUSE_NACK){
				//Command not acknowledged, resending
				return mouse_write_byte(mousebyte);
			} else if(response == MOUSE_ERROR){
				//Mouse error
				printf("mouse_write_byte::Mouse error!\n");
				return -3;
			}
		}

    //If this is not the case we just continue to another iteration after a small delay, retrying the operation
	  tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
  }

  printf("mouse_write_byte::The mouse byte could not be written after %d tries.\n", KBC_MAX_RETRIES);
  return -4;
}

int mouse_read_packetbyte(unsigned char *readbyte, unsigned long delay){
  unsigned int nretries;
	/*unsigned long statreg;
	unsigned long readOutbuf;*/

  //Due to the potential delay of the KBC, the read operation is attempted several times with a small delay
	for(nretries = 0; nretries <= KBC_MAX_RETRIES; ++nretries) {
/*
		if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
			//Error reading status register
			continue;
		}

		//If the i8042 output buffer is not empty (output buffer full, OBF flag is 1)
		if(statreg & KBC_OBF) {
			//If the data is not mouse input we exit straight away, returning 1 (indicates no output should be used)
			if((statreg & KBC_AUX) == 0) {
				return 1;
			}
			//If the status byte indicates that the data is valid, we will attempt to read the data
			if((statreg & (KBC_PARITY | KBC_TIMEOUT)) == 0){
				if(sys_inb(KBC_OUT_BUF, &readOutbuf) != OK){
					//Error reading output buffer
					return -1;
				} else {
					//Storing the data
					*readbyte = (unsigned char) readOutbuf;
					//Everything went as expected
					return 0;
				}
			} else {
				//Invalid data in OUT_BUF, parity or timeout error
				return -2;
			}
		}
		*/
		asm_readpacketbyte();
		printf("%c", asm_mouse_errorlevel);

		//The global variables are now altered

		if(asm_mouse_errorlevel != 0){
			//There was an error, returning the errorlevel (that is different than 0)
			return asm_mouse_errorlevel;
		}

		//Otherwise, there were no errors, returning 0 and passing the read scancode using the passed in variable
		*readbyte  =  (unsigned char )asm_readbyte;
		return 0;

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		//The delay is set by the calling function
		tickdelay(micros_to_ticks(delay));
	}

	//No data was read after the maximum number of retries (KBC_MAX_RETRIES)
	return -3;
}

int mouse_IH() {
	//Reading mouse IH
	if(mouse_receive_packet_IH() != 0){
		return -1;
	}

	if(mouse_packetbytecounter != 3) {
		//We do not send any event until there are 3 full packets read
		return 0;
	}

	//If 3 bytes were read, then a full packet is ready to be processed and the events to be sent
	//Resetting the counter (this can only be done here otherwise there would be no way to detect if a full packet was already received)
	mouse_packetbytecounter = 0;

	//Getting the pointer to the game object
	Robinix * rob = get_rob();

	//Sending event to game object if it exists
	if(rob != NULL){

		//Creating the move event (a move event is always sent, with the values of X and Y that the mouse was moved by)
		Event * move_evt = create_event(MOUSE_MOVE, mouse_get_X_movement(), mouse_get_Y_movement(), '?', NULL);

		//Sending the event (No need to check for NULL on creation since the event is not added if it is NULL)
		add_event_to_buffer(rob, move_evt);

		//Checking for clicks and sending respective events
		//We can create and send straight away since the event is not added if it is NULL
		//Left click
		if(mouse_is_LB_clicked()) {
			add_event_to_buffer(rob, create_event(MOUSE_LB_DOWN, 0, 0, '?', NULL));
		}
		//Middle click
		if(mouse_is_MB_clicked()) {
			add_event_to_buffer(rob, create_event(MOUSE_MB_DOWN, 0, 0, '?', NULL));
		}
		//Right click
		if(mouse_is_RB_clicked()) {
			add_event_to_buffer(rob, create_event(MOUSE_RB_DOWN, 0, 0, '?', NULL));
		}
	}

	//Regardless of adding event to buffer or not, everything went as expected
	return 0;
}

int mouse_receive_packet_IH(){
	//Read a byte with the default delay between retries
	unsigned char readbyte;
	int read_errorlevel = mouse_read_packetbyte(&readbyte, KBC_RETRY_DELAY);

	//Signifies that no input was received, so we do not process the input and jus exit
	if(read_errorlevel == 1) {
		return 0;
	}

	//-3 is a timeout error, which might not really be a problem
	//From my testing it is necessary to ignore the timeout errors because the first interrupt does not contain any data
	if(read_errorlevel != 0 && read_errorlevel != -3){
		//Error reading the packet byte
		return -1;
	}

	//Check byte sync
	//The first byte (byte 0 in our case) must always have bit 3 'active'
	if((mouse_packetbytecounter == 0) && ((readbyte & BIT(3)) == 0)){
		//If it doesn't (at byte 0 and bit 3 not active), there was a synchronization problem
		//Reset the counter to show that we are still at byte 0 probably
		mouse_packetbytecounter = 0;
		//Not resetting the byte array since it will be written over anyway

		//This is not an actual error so 0 can be returned
		//Since the global variables are affected the calling function knows how to deal with this (continue doing nothing)
		return 0;
	}

	//Update global variables since everything is ok
	mouse_packetarr[mouse_packetbytecounter] = readbyte;
	mouse_packetbytecounter++;

	//Everything went as expected
	return 0;
}

void mouse_print_packet(){
	//Printing "raw" bytes
	printf("B1=0x%02x B2=0x%02x B3=0x%02x", mouse_packetarr[0], mouse_packetarr[1], mouse_packetarr[2]);

	//Mouse buttons
	//Using %d to print the result of a condition (it will be 0 - false or 1 - true)
	//If the bit is active we want to display a 1.
	//1 appears when a condition is true, then when the bit is active the condition also must be true
	//Thus the condition must be checking if != 0
	printf(" LB=%d MB=%d RB=%d", (mouse_packetarr[0] & MOUSE_LB) != 0, (mouse_packetarr[0] & MOUSE_MB) != 0, (mouse_packetarr[0] & MOUSE_RB) != 0);

	//Overflow
	printf(" XOV=%d YOV=%d ", (mouse_packetarr[0] & MOUSE_XOVF) != 0, (mouse_packetarr[0] & MOUSE_YOVF) != 0);

	//X and Y deltas
	//X
	printf("X=");
	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_XSIGN){
		printf("%-4d ", (signed long) ((-1 << 8) | mouse_packetarr[1]));
	} else {
		printf("%-4d ", mouse_packetarr[1]);
	}

	//Y
	printf("Y=");
	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_YSIGN){
		printf("%-4d", (signed long) ((-1 << 8) | mouse_packetarr[2]));
	} else {
		printf("%-4d", mouse_packetarr[2]);
	}

	//Done, newline to finish
	printf("\n");
}
