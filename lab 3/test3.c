#include "test3.h"
#include "keyboard.h"
#include "i8042.h"
#include "timer.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
/* driver_receive appears to only be here, this should not be necessary */
#include <minix/driver.h>

///Global variable for sys_inb calls counting (defined in keyboard.h)
extern unsigned int SIB_COUNTER;
//Global variable for receiving the scancodes (defined in keyboard.h)
extern unsigned char scancode;
//Global variable for interrupt counting (defined in timer.c)
extern unsigned int timer_interrupt_counter;

int kbd_test_scan(unsigned short ass) {

	int irq_bitmask = keyboard_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;
	int is2byte = 0;
	int escapeNotYetPressed = 1;


	//Testing if the subscription went through
	if (irq_bitmask == -1 || irq_bitmask == -2) {
		printf("kbd_test_scan::Error subscribing the keyboard interrupt!\n");
		return -1 ;
	}

	while(escapeNotYetPressed == 1) {

		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("kbd_test_scan::Error in driver_receive() with: %d", driver_receive_errorlevel);
				continue;
		}

		//Verifying if the message received is a notification
		//(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
		if (is_ipc_notify(ipcStatus)) {
			switch (_ENDPOINT_P(msg.m_source)){
				//In the case that the notification is sent by the hardware
				case HARDWARE:
					//Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
					if(msg.NOTIFY_ARG & irq_bitmask) {
						//Using the C IH
						if(ass == 0){

							if(keyboard_read_scancode_IH() != 0){
								printf("kbd_test_scan::Error reading scancode with C IH\n");
								return -2;
							}
						} else {
							//Else, using the ASM IH
							if(keyboard_read_scancode_ass_IH() != 0){
								printf("kbd_test_scan::Error reading scancode with Assembly IH\n");
								return -2;
							}
						}

						//Analyzing the keyboard scancodes
						switch (keyboard_analyze_scancode(scancode)) {
							//Makecode
							case 0:
								if(is2byte){
									printf("Makecode:  0xe0 0x%02x\n", scancode);
									is2byte = 0;
								} else {
									printf("Makecode:  0x%02x\n", scancode);
								}
								break;
							//Breakcode
							case 1:
								if(is2byte){
									printf("Breakcode: 0xe0 0x%02x\n", scancode);
									is2byte = 0;
								} else {
									printf("Breakcode: 0x%02x\n", scancode);

									//If the scancode is a breakcode that is 1-byte long, it can be the code for the escape key release
									//If it is, break the loop
									if(scancode == ESC_BREAKCODE){
										printf("Escape breakcode received, exiting!\n");
										//break only breaks the switch statement so we set the loop flag to false
										escapeNotYetPressed = 0;
									}
								}
								break;
							//Start of 2-byte scancode
							case 2:
								is2byte = 1;
								break;
						}
					}
					break;
				//No other type of notification is expected, so we do nothing
				default:
					break;
			}
		}
	}

	//After exiting the loop show the amount of sys_inb operations, if using the C IH:
 	if (ass == 0) {
	 	printf("%d calls were made to sys_inb()\n", SIB_COUNTER);
 	}

	if (keyboard_unsubscribe_int() != 0) {
		printf("kbd_test_scan::Error unsubscribing from the keyboard interrupt!\n");
		return -3;
	}

	//Everything went as expected
	return 0;
}

int kbd_test_poll() {

	int scancode_read_errorlevel = 0;
	int is2byte = 0;

	while (scancode != ESC_BREAKCODE) {
		//Attempting to read the scancode
		scancode_read_errorlevel = keyboard_read_scancode_poll();
		/* Since -4 only means that no codes were read after KBC_MAX_RETRIES number of retries,
		 * there is no problem in the read operation, the OUT_BUF is just empty.
		 * Thus, we ignore that state when checking for errors,
		 * even continuing to a new iteration, since no new data was read.
		*/
		if(scancode_read_errorlevel == -4){
			//No data was read, retry
			continue;
		}

		if(scancode_read_errorlevel != 0 && scancode_read_errorlevel != -4){
			printf("kbd_test_poll::Error reading scancode\n");
			return -1;
		}

		//Analyzing the keyboard scancodes
		switch (keyboard_analyze_scancode(scancode)) {
			//Makecode
			case 0:
				if(is2byte){
					printf("Makecode:  0xe0 0x%02x\n", scancode);
					is2byte = 0;
				} else {
					printf("Makecode:  0x%02x\n", scancode);
				}
				break;
			//Breakcode
			case 1:
				if(is2byte){
					printf("Breakcode: 0xe0 0x%02x\n", scancode);
					is2byte = 0;
				} else {
					printf("Breakcode: 0x%02x\n", scancode);

					//If the scancode is a breakcode that is 1-byte long, it can be the code for the escape key release
					//If it is, break the loop
					if(scancode == ESC_BREAKCODE){
						printf("Escape breakcode received, exiting!\n");
						//The scancode received is checked at the start of each iteration of the while loop so no extra breaking is necessary
					}
				}
				break;
			//Start of 2-byte scancode
			case 2:
				is2byte = 1;
				break;
		}
	}

	//After exiting the loop show the amount of sys_inb operations:
 	printf("%d calls were made to sys_inb()\n", SIB_COUNTER);

	//Reactivating interrupts
	//1 - reading current command byte
	unsigned char cbyte;
	if(keyboard_read_cmdbyte(&cbyte) != 0){
		printf("kbd_test_poll::Error reading command byte\n");
		return -2;
	}
	//2 - calculating the new command byte to write, that has the bit that activates interrupts active
	cbyte |= KBC_CB_INT;
	//3 - writing the new command byte
	if (keyboard_write_cmdbyte(cbyte) != 0) {
		printf("kbd_test_poll::Error writing new command byte\n");
		return -3;
	}

	//Everything went as expected
	return 0;
}

int kbd_test_timed_scan(unsigned short n) {

	int keyboard_irq_bitmask = keyboard_subscribe_int();
	int timer_irq_bitmask = timer_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;
	int is2byte = 0;
	unsigned int timer_int_max = n * 60;

	//Testing if the keyboard subscription went through
	if (keyboard_irq_bitmask == -1 || keyboard_irq_bitmask == -2) {
		printf("kbd_test_timed_scan::Failed to subscribe the keyboard!\n");
		return -1;
	}

	//Testing if the timer subscription went through
	if (timer_irq_bitmask == -1 || timer_irq_bitmask == -2){
		printf ("kbd_test_timed_scan::Failed to subscribe the timer! \n");
		return -2;
	}

	while(timer_interrupt_counter < timer_int_max && scancode != ESC_BREAKCODE) {

		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("kbd_test_timed_scan::Error in driver_receive() with: %d", driver_receive_errorlevel);
				continue;
		}

		//Verifying if the message received is a notification
		//(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
		if (is_ipc_notify(ipcStatus)) {
			switch (_ENDPOINT_P(msg.m_source)){
				//In the case that the notification is sent by the hardware
				case HARDWARE:
					//Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
					if(msg.NOTIFY_ARG & keyboard_irq_bitmask) {

						if(keyboard_read_scancode_IH() != 0){
							printf("kbd_test_scan::Error reading scancode\n");
							return -2;
						}

						//On keyboard input, reset the number of interrupts that have ocurred without keyboard input
						timer_interrupt_counter = 0;

						//Analyzing the keyboard scancodes
						switch (keyboard_analyze_scancode(scancode)) {
							//Makecode
							case 0:
								if(is2byte){
									printf("Makecode:  0xe0 0x%02x\n", scancode);
									is2byte = 0;
								} else {
									printf("Makecode:  0x%02x\n", scancode);
								}
								break;
							//Breakcode
							case 1:
								if(is2byte){
									printf("Breakcode: 0xe0 0x%02x\n", scancode);
									is2byte = 0;
								} else {
									printf("Breakcode: 0x%02x\n", scancode);
								}
								break;
							//Start of 2-byte scancode
							case 2:
								is2byte = 1;
								break;
						}
					}
					if(msg.NOTIFY_ARG & timer_irq_bitmask){
						//Calling the timer interrupt handler that counts the number of interrupts
						timer_int_handler();
					}
					break;
				//No other type of notification is expected, so we do nothing
				default:
					break;
			}
		}
	}

	//Break was due to receiving escape key breakcode
	if(scancode == ESC_BREAKCODE){
		printf("Escape breakcode received, exiting!\n");
	} else{
		printf("%d seconds elapsed without receiving scancodes, terminating kbd_test_timed_scan...\n", n);
	}

	//Unsubscribing from the timer interrupt
	if(timer_unsubscribe_int() != OK){
		printf("kbd_test_timed_scan::Error unsubscribing from the timer interrupt!\n");
		return -3;
	}

	//Unsubscribing from the keyboard interrupt
	if (keyboard_unsubscribe_int() != 0) {
		printf("kbd_test_timed_scan::Error unsubscribing from the keyboard interrupt!\n");
		return -4;
	}

	//Everything went as expected
	return 0;
}
