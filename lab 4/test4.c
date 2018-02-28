#include "test4.h"
#include "mouse.h"
#include "i8042.h"
#include "keyboard.h"
#include "timer.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/driver.h>

//Global variables for receiving mouse packets (both defined in mouse.c)
//Array of received bytes, if there are 3 then a full packet was received and can be printed
extern unsigned char mouse_packetarr[3];
//Keeps track of which byte of the packet we are at (goes from 0 to 2)
extern unsigned int mouse_packetbytecounter;
//Global variable for interrupt counting (defined in timer.c)
extern unsigned int timer_interrupt_counter;
//Global variable for the delay between mouse byte reads in remote mode (defined in mouse.c)
extern unsigned long mouse_remote_delay;

//Global variable that holds the current state
static state_st CURRSTATE = {0, 0, INIT};

int mouse_test_packet(unsigned short cnt){
	//Variables:
	int irq_bitmask = mouse_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;
	unsigned short npackets = cnt;

	//Testing if the subscription went through
	if (irq_bitmask == -1 || irq_bitmask == -2) {
		printf("mouse_test_packet::Error subscribing the mouse interrupt!\n");
		return -1 ;
	}


	//Enabling stream mode since it is disabled in text mode
	if(mouse_write_byte(MOUSE_SET_STREAMMODE) != 0){
		printf("mouse_test_packet::Error setting stream mode!\n");
		return -2;
	}


	//Enabling data reporting
	if(mouse_write_byte(MOUSE_ENABLE) != 0){
		printf("mouse_test_packet::Error enabling data reporting!\n");
		return -3;
	}

	while(npackets != 0) {
		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("mouse_test_packet::Error in driver_receive() with: %d", driver_receive_errorlevel);
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
						//Calling the IH
						if(mouse_receive_packet_IH() != 0){
							printf("mouse_test_packet::Error receiving mouse packet byte!\n");
							return -4;
						}

						//If a full packet has been received, print it and reset the counter
						//(Counter goes from 0 to 2, so this means that 3 bytes have been received)
						if(mouse_packetbytecounter == 3){
							mouse_print_packet();
							mouse_packetbytecounter = 0;
							//Also count down the number of packets that we need to receive still
							npackets--;
						}

					}
					break;
				//No other type of notification is expected, so we do nothing
				default:
					break;
			}
		}
	}

	//Disabling data reporting
	if(mouse_write_byte(MOUSE_DISABLE) != 0){
		printf("mouse_test_packet::Error disabling data reporting!\n");
		return -5;
	}

	if(mouse_unsubscribe_int() != 0){
		printf("mouse_test_packet::Error unsubscribing from the mouse interrupt!\n");
		return -6;
	}

	//Clearing the OUT_BUF because if something is left there the KBC might not generate further interrupts
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_test_packet::Error clearing OUT_BUF!\n");
		return -7;
	}

	//Everything went as expected
	printf("Function terminated after receiving %u packets.\n", cnt);
	return 0;
}

int mouse_test_async(unsigned short idle_time){

	int mouse_irq_bitmask = mouse_subscribe_int();
	int timer_irq_bitmask = timer_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;
	unsigned int timer_int_max = idle_time * 60;

	//Testing if the mouse subscription went through
	if (mouse_irq_bitmask == -1 || mouse_irq_bitmask == -2) {
		printf("mouse_test_async::Error subscribing the mouse interrupt!\n");
		return -1 ;
	}

	//Testing if the timer subscription went through
	if (timer_irq_bitmask == -1 || timer_irq_bitmask == -2){
		printf ("mouse_test_async::Failed to subscribe the timer! \n");
		return -2;
	}

	//Enabling stream mode since it is disabled in text mode
	if(mouse_write_byte(MOUSE_SET_STREAMMODE) != 0){
		printf("mouse_test_async::Error setting stream mode!\n");
		return -2;
	}

	//Enabling data reporting
	if(mouse_write_byte(MOUSE_ENABLE) != 0){
		printf("mouse_test_async::Error enabling data reporting!\n");
		return -3;
	}

	while(timer_interrupt_counter < timer_int_max) {
		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("mouse_test_async::Error in driver_receive() with: %d", driver_receive_errorlevel);
				continue;
		}

		//Verifying if the message received is a notification
		//(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
		if (is_ipc_notify(ipcStatus)) {
			switch (_ENDPOINT_P(msg.m_source)){
				//In the case that the notification is sent by the hardware
				case HARDWARE:
					//Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
					if(msg.NOTIFY_ARG & mouse_irq_bitmask) {
						//Calling the IH
						if(mouse_receive_packet_IH() != 0){
							printf("mouse_test_async::Error receiving mouse packet byte!\n");
							return -4;
						}

						//On mouse input, reset the number of interrupts that have ocurred without mouse input
						timer_interrupt_counter = 0;

						//If a full packet has been received, print it and reset the counter
						//(Counter goes from 0 to 2, so this means that 3 bytes have been received)
						if(mouse_packetbytecounter == 3){
							mouse_print_packet();
							mouse_packetbytecounter = 0;
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

	//Disabling data reporting
	if(mouse_write_byte(MOUSE_DISABLE) != 0){
		printf("mouse_test_async::Error disabling data reporting!\n");
		return -5;
	}

	if(mouse_unsubscribe_int() != 0){
		printf("mouse_test_async::Error unsubscribing from the mouse interrupt!\n");
		return -6;
	}

	//Clearing the OUT_BUF because if something is left there the KBC might not generate further interrupts
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_test_async::Error clearing OUT_BUF!\n");
		return -7;
	}

	//Everything went as expected
	printf("Function terminated after %u seconds without mouse input.\n", idle_time);
	return 0;
}

int mouse_test_remote(unsigned long period, unsigned short cnt){

	//Disabling Minix's default IH
	if(mouse_disable_minixIH() != 0){
		printf("mouse_test_remote::Error disabling default mouse IH!\n");
		return -1;
	}

	//Enabling data reporting before disabling so that everything works correctly
	//This is for some reason necessary for this function to work correctly
	if(mouse_write_byte(MOUSE_ENABLE) != 0){
		printf("mouse_test_async::Error enabling data reporting!\n");
		return -2;
	}

	//Disabling mouse data reporting
	if(mouse_write_byte(MOUSE_DISABLE) != 0){
		printf("mouse_test_remote::Error disabling data reporting!\n");
		return -3;
	}

	//Setting remote mode
	if(mouse_write_byte(MOUSE_SET_REMOTEMODE) != 0){
		printf("mouse_test_remote::Error setting remote mode!\n");
		return -4;
	}

	//Setting the delay (by changing the global variable)
	mouse_remote_delay = period;

	while(cnt != 0){

		if(mouse_receive_packet_remote() != 0){
			printf("mouse_test_remote::Error reading mouse packet byte!\n");
			return -5;
		}

		//If a full packet has been received, print it and reset the counter
		//(Counter goes from 0 to 2, so this means that 3 bytes have been received)
		if(mouse_packetbytecounter == 3){
			mouse_print_packet();
			mouse_packetbytecounter = 0;
			//Also count down the number of packets that we need to receive still
			cnt--;
		}
	}

	//Setting stream mode once again
	if(mouse_write_byte(MOUSE_SET_STREAMMODE) != 0){
		printf("mouse_test_remote::Error setting stream mode once again!\n");
		return -6;
	}

	//Ensuring data reporting is disabled
	if(mouse_write_byte(MOUSE_DISABLE) != 0){
		printf("mouse_test_remote::Error disabling data reporting!\n");
		return -7;
	}

	//Clearing the OUT_BUF because if something is left there the KBC might not generate further interrupts
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_test_remote::Error clearing OUT_BUF!\n");
		return -8;
	}

	//Reenabling Minix's default IH
	if(mouse_enable_minixIH() != 0){
		printf("mouse_test_remote::Error reenabling default mouse IH!\n");
		return -9;
	}

	//Everything went as expected
	printf("Function terminated after receiving the given number of packets.\n");
	return 0;
}

int mouse_test_gesture(short length){
	//Test to see if the length is a non-null number
	if (length == 0) {
		printf("mouse_test_gesture::Error in the passed parameter, length has to be different from 0!\n");
		return -1;
	}

	//Variables
	int irq_bitmask = mouse_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;

	//Ensuring that the starting curent state is the initial one
	CURRSTATE.state = INIT;
	//Initializing with the correct goal and progress
	CURRSTATE.objective = length;
	CURRSTATE.progress = 0;

	//Testing if the subscription went through
	if (irq_bitmask == -1 || irq_bitmask == -2) {
		printf("mouse_test_gesture::Error subscribing the mouse interrupt!\n");
		return -2 ;
	}

	//Enabling stream mode since it is disabled in text mode
	if(mouse_write_byte(MOUSE_SET_STREAMMODE) != 0){
		printf("mouse_test_gesture::Error setting stream mode!\n");
		return -3;
	}


	//Enabling data reporting
	if(mouse_write_byte(MOUSE_ENABLE) != 0){
		printf("mouse_test_gesture::Error enabling data reporting!\n");
		return -4;
	}

	while(CURRSTATE.state != COMPLETED) {
		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("mouse_test_gesture::Error in driver_receive() with: %d", driver_receive_errorlevel);
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
						//Calling the IH
						if(mouse_receive_packet_IH() != 0){
							printf("mouse_test_gesture::Error receiving mouse packet byte!\n");
							return -5;
						}

						//If a full packet has been received, print it and reset the counter
						//(Counter goes from 0 to 2, so this means that 3 bytes have been received)
						if(mouse_packetbytecounter == 3){
							mouse_print_packet();
							//Checking if events ocurred and calling the function that updates the states based on events detected
							mouse_update_events();
							mouse_packetbytecounter = 0;
						}

					}
					break;
				//No other type of notification is expected, so we do nothing
				default:
					break;
			}
		}
	}

	//Disabling data reporting
	if(mouse_write_byte(MOUSE_DISABLE) != 0){
		printf("mouse_test_gesture::Error disabling data reporting!\n");
		return -6;
	}

	if(mouse_unsubscribe_int() != 0){
		printf("mouse_test_gesture::Error unsubscribing from the mouse interrupt!\n");
		return -7;
	}

	//Clearing the OUT_BUF because if something is left there the KBC might not generate further interrupts
	if(keyboard_clear_outbuf() != 0){
		printf("mouse_test_gesture::Error clearing OUT_BUF!\n");
		return -8;
	}

	//Everything went as expected
	printf("Function terminated after receiving the gesture.\n");
	return 0;
}

void mouse_update_state(event_st * received_evt){
	switch (CURRSTATE.state) {
		case INIT:
			if(received_evt->event == RB_DOWN){
				CURRSTATE.state = DRAWING;
			}
			break;
		case DRAWING:
			if(received_evt->event == RB_UP){
				CURRSTATE.state = INIT;
				CURRSTATE.progress = 0;
			}else if(received_evt->event == MOVE){
				//Verifying if there is a positive slope (x and y have equal signs)
				if(received_evt->x * received_evt->y > 0){
					CURRSTATE.progress += received_evt->x;
				} else {
					//Reset the progress since the drawing is not an upwards slope
					CURRSTATE.progress = 0;
				}
			}
			//Verifying if desired length has been reached
			//If the objective is negative
			if(CURRSTATE.objective > 0){
				if(CURRSTATE.progress >= CURRSTATE.objective){
					CURRSTATE.state = COMPLETED;
				}
				//If the objective is positive
			} else if(CURRSTATE.objective < 0){
				if(CURRSTATE.progress <= CURRSTATE.objective){
					CURRSTATE.state = COMPLETED;
				}
			}
			break;
		//COMPLETED not expected here, as soon as it is set the program will exit
		//However, to avoid errors we add the case to do nothing
		case COMPLETED:
			break;
	}
}

void mouse_update_events(){
	static event_st evt;

	//Detecting RB_DOWN event
	if(mouse_packetarr[0] & MOUSE_RB){
		evt.event = RB_DOWN;
		mouse_update_state(&evt);
	}

	//Detecting RB_UP event
	if((mouse_packetarr[0] & MOUSE_RB) == 0){
		evt.event = RB_UP;
		mouse_update_state(&evt);
	}

	//"Detecting" MOVE event (it always occurrs, so it is always sent to the state updating function)
	static event_st move_evt;
	move_evt.event = MOVE;
	//X
	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_XSIGN){
		 move_evt.x = ((-1 << 8) | mouse_packetarr[1]);
	} else {
		move_evt.x = mouse_packetarr[1];
	}
	//Y
	//If the sign bit is 1, the value is negative and must be interpreted as such
	if(mouse_packetarr[0] & MOUSE_YSIGN){
		move_evt.y = ((-1 << 8) | mouse_packetarr[2]);
	} else {
		move_evt.y = mouse_packetarr[2];
	}

	mouse_update_state(&move_evt);
}
