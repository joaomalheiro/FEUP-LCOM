#include "mouse.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
/* for tickdelay and micros_to_ticks */
#include <minix/sysutil.h>
#include "i8042.h"
//Including the keyboard functions because of reusing code to send commands to the KBC
#include "keyboard.h"

////Global variables
//Hook ID for subscribing to mouse interrupts
int mouse_hookID = 2;

//Global varaibles for receiving mouse packets
//Array of received bytes, if there are 3 then a full packet was received and can be printed
unsigned char mouse_packetarr[3];
//Keeps track of which byte of the packet we are at (goes from 0 to 2)
unsigned int mouse_packetbytecounter = 0;
//Global variable for the delay between mouse byte reads in remote mode
unsigned long mouse_remote_delay;

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
			sys_outb(KBC_IN_BUF, mousebyte);

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
	unsigned long statreg;
	unsigned long readOutbuf;

  //Due to the potential delay of the KBC, the read operation is attempted several times with a small delay
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

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		//The delay is set by the calling function
		tickdelay(micros_to_ticks(delay));
	}

	//No data was read after the maximum number of retries (KBC_MAX_RETRIES)
	return -3;
}

int mouse_receive_packet_IH(){
	//Read a byte with the default delay between retries
	unsigned char readbyte;
	int read_errorlevel = mouse_read_packetbyte(&readbyte, KBC_RETRY_DELAY);

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

int mouse_receive_packet_remote(){

	//Requesting data reading by sending the READ_DATA command to the mouse
	if(mouse_write_byte(MOUSE_READ_DATA) != 0){
		printf("mouse_receive_packet_remote::Error sending READ_DATA command to the mouse\n");
		return -1;
	}

	unsigned int i = 0;

	while (i < 3) {
		//Reading a byte with the given delay (passed as a global variable by the calling function - mouse_remote_delay)
		unsigned char readbyte;
		//*1000 because the function receives micros
		int reading_errorlevel = mouse_read_packetbyte(&readbyte, mouse_remote_delay*1000);
		//-3 is not an error, it just signifies that no bytes were read until the maximum number of retries, which is not an error when in remote mode
		if(reading_errorlevel != 0 && reading_errorlevel != -3){
			printf("mouse_receive_packet_remote::Error reading mouse packet byte!\n");
			return -2;
		}

		//Delay because if successful read straight away there is no delay between reads (was what was happenning)
		//*1000 because the function receives micros
		tickdelay(micros_to_ticks(mouse_remote_delay*1000));

		//Check byte sync
		//The first byte (byte 0 in our case) must always have bit 3 'active'
		if((mouse_packetbytecounter == 0) && ((readbyte & BIT(3)) == 0)){
			//If it doesn't (at byte 0 and bit 3 not active), there was a synchronization problem
			//Reset the counter to show that we are still at byte 0 probably
			mouse_packetbytecounter = 0;
			//Not resetting the byte array since it will be written over anyway

			//This is not an actual error so 0 can be returned
			//Since the global variables are affected the calling function knows how to deal with this (continue doing nothing)
			continue;
		}

		//Update global variables since everything is ok
		mouse_packetarr[mouse_packetbytecounter] = readbyte;
		mouse_packetbytecounter++;
		i++;
	}

	//Everything went as expected
	return 0;
}

int mouse_disable_minixIH(){

	//Disabling minix IH by writing to the KBC's command byte
	//However, to prevent minix's keyboard IH from 'stealing' content from the OUT_BUF,
	//we must temporarily subscribe the keyboard interrupts!
	int keyboard_irq = keyboard_subscribe_int();
	if(keyboard_irq == -1 || keyboard_irq == -2){
		printf("mouse_disable_minixIH::Error subscribing keyboard interrupts");
		return -1;
	}

	//1 - reading current command byte
	unsigned char cbyte;
	if(keyboard_read_cmdbyte(&cbyte) != 0){
		printf("mouse_disable_minixIH::Error reading command byte to disable interrupts\n");
		return -2;
	}
	//2 - calculating the new command byte to write, that has the bit that activates interrupts inactive
	//(To 'deactivate' a bit, we must perform an AND with the NOT of the bit to 'unset')
	cbyte &= ~KBC_CB_INT2;
	//3 - writing the new command byte
	if (keyboard_write_cmdbyte(cbyte) != 0) {
		printf("mouse_disable_minixIH::Error writing new command byte to disable interrupts\n");
		return -3;
	}

	//Undoing the keyboard subscription, as it is no longer necessary
	if(keyboard_unsubscribe_int() != 0){
		printf("mouse_disable_minixIH::Error unsubscribing keyboard interrupts!\n");
		return -4;
	}

	//Clearing the OUT_BUF because there might be something left there by user inputs
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_disable_minixIH::Error clearing OUT_BUF!\n");
		return -5;
	}

	//Everything went as expected
	return 0;
}

int mouse_enable_minixIH(){
	//(Re)enabling minix IH by writing to the KBC's command byte
	//However, to prevent minix's keyboard IH from 'stealing' content from the OUT_BUF,
	//we must temporarily subscribe the keyboard interrupts!
	int keyboard_irq = keyboard_subscribe_int();
	if(keyboard_irq == -1 || keyboard_irq == -2){
		printf("mouse_disable_minixIH::Error subscribing keyboard interrupts");
		return -1;
	}

	//1 - reading current command byte
	unsigned char cbyte;
	if(keyboard_read_cmdbyte(&cbyte) != 0){
		printf("mouse_disable_minixIH::Error reading command byte to enable interrupts\n");
		return -2;
	}
	//2 - calculating the new command byte to write, that has the bit that activates interrupts active
	cbyte |= KBC_CB_INT2;
	//3 - writing the new command byte
	if (keyboard_write_cmdbyte(cbyte) != 0) {
		printf("mouse_disable_minixIH::Error writing new command byte to enable interrupts\n");
		return -3;
	}

	//Undoing the keyboard subscription, as it is no longer necessary
	if(keyboard_unsubscribe_int() != 0){
		printf("mouse_disable_minixIH::Error unsubscribing keyboard interrupts!\n");
		return -4;
	}

	//Clearing the OUT_BUF because there might be something left there by user inputs
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_disable_minixIH::Error clearing OUT_BUF!\n");
		return -5;
	}

	//Everything went as expected
	return 0;
}
