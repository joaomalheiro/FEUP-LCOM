#include <minix/syslib.h>
#include <minix/drivers.h>
/* for tickdelay and micros_to_ticks */
#include <minix/sysutil.h>
#include "keyboard.h"
#include "i8042.h"

//Hook ID for subscribing to keyboard interrupts
int kb_hookID = 1;

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

int keyboard_read_scancode_IH(){
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
			//If the status byte indicates that the data is valid and if the data isn't mouse input, we will attempt to read the data
			if((statreg & (KBC_PARITY | KBC_TIMEOUT | KBC_AUX )) == 0){
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

		//If this is not the case we just continue to another iteration after a small delay, retrying the read operation
		tickdelay(micros_to_ticks(KBC_RETRY_DELAY));
	}

  //Removed printf statement due to it generating a large amount of unnecessary messages when using polling
	//printf("keyboard_read_scancode::No data was read after %d tries.\n", KBC_MAX_RETRIES);
	return -3;
}

int keyboard_read_scancode_poll(){
	unsigned long statreg;
	unsigned long readOutbuf;

  //The delay is to prevent making an exaggerated number of operations
  tickdelay(micros_to_ticks(KBC_RETRY_DELAY));

	if(sys_inb(KBC_STATUS_REG, &statreg) != OK){
		//Error reading status register
		return -1;
	}

	//If the i8042 output buffer is not empty (output buffer full, OBF flag is 1)
	if(statreg & KBC_OBF) {
		//If the status byte indicates that the data is valid and if the data isn't mouse input,
    //we will attempt to read the data
		if((statreg & (KBC_PARITY | KBC_TIMEOUT | KBC_AUX )) == 0){
			if(sys_inb(KBC_OUT_BUF, &readOutbuf) != OK){
				printf("keyboard_read_scancode::Error reading output buffer\n");
				return -2;
			} else{
				//Storing the data
				scancode = (unsigned char) readOutbuf;
				//Everything went as expected
				return 0;
			}
		} else {
			printf("keyboard_read_scancode::Invalid data in OUT_BUF, parity or timeout error\n");
			return -3;
		}
	} else {
    //In the eventuality that the OBF flag is not set
    //printf removed because it caused unnecessary spam (and made the actual output unreadable)
    //printf("keyboard_read_scancode::OBF flag not set!\n");
    return -4;
  }
}

int keyboard_analyze_scancode(unsigned char scancode){

	if(scancode == 0xe0)
		return 2;

	if((scancode & BIT(7)) == 0)
		return 0;
	else
		return 1;
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
      sys_outb(KBC_CMD_REG, kbccmd);
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
      sys_outb(KBC_IN_BUF, cmdbyte);

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
