#include "test5.h"
#include "video_gr.h"
#include "keyboard.h"
#include "i8042.h"
#include "i8254.h"
#include "timer.h"
#include "video_test.h"
#include "sprite.h"
#include "vbe.h"
#include <math.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/driver.h>

//Global variable for receiving the scancodes (defined in keyboard.h)
extern unsigned char scancode;

int video_test_init(unsigned short mode, unsigned short delay) {

    if(vg_init(mode) == NULL){
      printf("video_test_init::Error initializing video mode\n");
      return -1;
    }

    sleep(delay);

    if(vg_exit() == -1){
      printf("video_test_init::Error exiting video mode\n");
      return -2;
    }

    //Everything went as expected
	  return 0;
}

int video_test_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {

  int irq_bitmask = keyboard_subscribe_int();
	int driver_receive_errorlevel;
	int ipcStatus;
	message msg;
	int escapeNotYetPressed = 1;

	//Testing if the subscription went through
	if (irq_bitmask == -1 || irq_bitmask == -2) {
		printf("video_test_square::Error subscribing the keyboard interrupt!\n");
		return -1 ;
	}

  if(vg_init(0x105) == NULL){
    printf("video_test_square::Error initializing video mode\n");
    return -2;
  }

  //Drawing a rectangle (requested square since the sizes are equal) based on the given arguments
  /* Considering the rectangle that starts at (xi, yi) and ends at (xf, yf), the coordinates for such rectangle, based on the ones for the square are:
   * xi = HRES/2 + centerx - size/2
   * yi = VRES/2 + centery - size/2
   * xf = HRES/2 + centerx + size/2
   * yf = VRES/2 + centery + size/2
   */
  vg_draw_rect(HRES/2 + x - size/2, VRES/2 + y - size/2, HRES/2 + x + size/2, VRES/2 + y + size/2, color);

  //Invoking automatic grading function
  video_dump_fb();

	while(escapeNotYetPressed) {

		//Receiving messages from the DD
		if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
				printf("video_test_square::Error in driver_receive() with: %d", driver_receive_errorlevel);
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

            //Reading the keyboard on a keyboard interrupt
						if(keyboard_read_scancode_IH() != 0){
							printf("video_test_square::Error reading scancode with C IH\n");
							return -3;
						}

            //If the scancode received is
            if(scancode == ESC_BREAKCODE){
              printf("Escape breakcode received, exiting!\n");
              //break only breaks the switch statement so we set the loop flag to false
              escapeNotYetPressed = 0;
            }

					}
					break;
				//No other type of notification is expected, so we do nothing
				default:
					break;
			}
		}
	}

	if (keyboard_unsubscribe_int() != 0) {
		printf("video_test_square::Error unsubscribing from the keyboard interrupt!\n");
		return -4;
	}

  if(vg_exit() == -1){
    printf("video_test_square::Error exiting video mode\n");
    return -5;
  }

	//Everything went as expected
	return 0;
}

int video_test_line(unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf, unsigned long color) {

  int irq_bitmask = keyboard_subscribe_int();
  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;
  int escapeNotYetPressed = 1;

 	//Testing if the subscription went through
 	if (irq_bitmask == -1 || irq_bitmask == -2){
     printf("video_test_line::Error subscribing the keyboard interrupt!\n");
     return -1 ;
  }

  if(vg_init(0x105) == NULL){
    printf("video_test_line::Error initializing video mode\n");
    return -2;
  }

  vg_draw_line(xi,yi,xf,yf,color);

  //Invoking automatic grading function
  video_dump_fb();

  while(escapeNotYetPressed) {
    //Receiving messages from the DD
    if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
      printf("video_test_line::Error in driver_receive() with: %d", driver_receive_errorlevel);
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
            //Reading the keyboard on a keyboard interrupt
            if(keyboard_read_scancode_IH() != 0){
              printf("video_test_line::Error reading scancode with C IH\n");
              return -3;
            }

            //If the scancode received is
            if(scancode == ESC_BREAKCODE){
              printf("Escape breakcode received, exiting!\n");
              //break only breaks the switch statement so we set the loop flag to false
              escapeNotYetPressed = 0;
            }
          }
          break;
        //No other type of notification is expected, so we do nothing
        default:
          break;
      }
    }
  }

  if (keyboard_unsubscribe_int() != 0) {
    printf("video_test_line::Error unsubscribing from the keyboard interrupt!\n");
    return -4;
  }

  if(vg_exit() == -1){
    printf("video_test_line::Error exiting video mode\n");
    return -5;
  }

  //Everything went as expected
 	return 0;
}

int video_test_xpm(char *xpm[], unsigned short xi, unsigned short yi) {

  int irq_bitmask = keyboard_subscribe_int();
  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;
  int escapeNotYetPressed = 1;

  //Testing if the subscription went through
  if (irq_bitmask == -1 || irq_bitmask == -2){
     printf("video_test_xpm::Error subscribing the keyboard interrupt!\n");
     return -1 ;
  }

  if(vg_init(0x105) == NULL){
    printf("video_test_xpm::Error initializing video mode\n");
    return -2;
  }

  vg_draw_xpm(xpm,xi,yi);
  //Invoking automatic grading function
  video_dump_fb();

  while(escapeNotYetPressed) {

   //Receiving messages from the DD
   if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
     printf("video_test_xpm::Error in driver_receive() with: %d", driver_receive_errorlevel);
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

           //Reading the keyboard on a keyboard interrupt
           if(keyboard_read_scancode_IH() != 0){
              printf("video_test_xpm::Error reading scancode with C IH\n");
              return -3;
              }

           //If the scancode received is
           if(scancode == ESC_BREAKCODE){
             printf("Escape breakcode received, exiting!\n");
             //break only breaks the switch statement so we set the loop flag to false
             escapeNotYetPressed = 0;
             }
           }
         break;
         //No other type of notification is expected, so we do nothing
       default:
       break;
       }
     }
   }

   if (keyboard_unsubscribe_int() != 0) {
     printf("video_test_xpm::Error unsubscribing from the keyboard interrupt!\n");
     return -4;
   }

   if(vg_exit() == -1){
     printf("video_test_xpm::Error exiting video mode\n");
     return -5;
   }

   //Everything went as expected
   return 0;
}

int video_test_move(char *xpm[], unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf, short s, unsigned short f) {

  if (f > 60){
    f = 60;
    printf("video_test_move::The maximum refresh rate supported is 60 frames per second. It is now 60 fps \n");
  }
  if (s == 0) {
    printf("video_test_move::Invalid speed argument! It can't be 0 \n");
  }

  int kb_irq_bitmask = keyboard_subscribe_int();
  int timer_irq_bitmask = timer_subscribe_int();
  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;
  int escapeNotYetPressed = 1;
  int finishMovFlag = 1;
  int speedX;
  int speedY;
  int nFrames_between_updates = 1;

  //Testing if the keyboard subscription went through
  if (kb_irq_bitmask == -1 || kb_irq_bitmask == -2){
     printf("video_test_move::Error subscribing the keyboard interrupt!\n");
     return -1 ;
  }

  //Testing if the timer subscription went through
  if (timer_irq_bitmask == -1 || timer_irq_bitmask == -2){
     printf("video_test_move::Error subscribing the timer interrupt!\n");
     return -1 ;
  }

  if(vg_init(0x105) == NULL){
    printf("video_test_move::Error initializing video mode\n");
    return -2;
  }

  //Calculating the speed
  if (s > 0) {
    if (abs(xf-xi) > abs(yf-yi)) {
      //The division serves as a way to retrieve the sign of delta_x
      speedX = s * ((xf-xi) / abs(xf-xi));
      speedY = 0;
    } else {
      //The division serves as a way to retrieve the sign of delta_y
      speedY = s * ((yf-yi) / abs(yf-yi));
      speedX = 0;
    }
  } else {
    nFrames_between_updates = abs(s);
    if (abs(xf-xi) > abs(yf-yi)) {
      //The division serves as a way to retrieve the sign of delta_x
      speedX = 1 * ((xf-xi) / abs(xf-xi));
      speedY = 0;
    } else {
      //The division serves as a way to retrieve the sign of delta_y
      speedY = 1 * ((yf-yi) / abs(yf-yi));
      speedX = 0;
    }
  }

  //Calculating the number of timer interrupts to draw a frame
  int n_int_to_frame = TIMER_DEFAULT_FREQ / f;

  //Creating the sprite
  Sprite* sprite_p = create_sprite(xpm, xi, yi, speedX, speedY);

  while(escapeNotYetPressed && finishMovFlag) {

   //Receiving messages from the DD
   if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
     printf("video_test_move::Error in driver_receive() with: %d", driver_receive_errorlevel);
     continue;
   }

   //Verifying if the message received is a notification
   //(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
   if (is_ipc_notify(ipcStatus)) {
     switch (_ENDPOINT_P(msg.m_source)){
       //In the case that the notification is sent by the hardware
       case HARDWARE:
       //Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & kb_irq_bitmask) {
           //Reading the keyboard on a keyboard interrupt
           if(keyboard_read_scancode_IH() != 0){
              printf("video_test_move::Error reading scancode with C IH\n");
              return -3;
              }

           //If the scancode received is
           if(scancode == ESC_BREAKCODE){
             printf("Escape breakcode received, exiting!\n");
             //break only breaks the switch statement so we set the loop flag to false
             escapeNotYetPressed = 0;
             }
           }
         if(msg.NOTIFY_ARG & timer_irq_bitmask){
           //Since only when we receive a timer interrupt the time can be right to draw
           //We only test inside here if that is the case
           if(timer_interrupt_counter % n_int_to_frame == 0) {
             //If this is true, in this frame we are supposed to draw
             finishMovFlag = sprite_move(sprite_p, xf, yf, nFrames_between_updates);
           }

           timer_int_handler();
         }
         break;
         //No other type of notification is expected, so we do nothing
       default:
       break;
       }
     }
   }

   if (keyboard_unsubscribe_int() != 0) {
     printf("video_test_move::Error unsubscribing from the keyboard interrupt!\n");
     return -4;
   }

   if(timer_unsubscribe_int() != 0){
     printf("video_test_move::Error unsubscribing from the keyboard interrupt!\n");
     return -5;
   }

   destroy_sprite(sprite_p);

   if(vg_exit() == -1){
     printf("video_test_move::Error exiting video mode\n");
     return -6;
   }

   //Everything went as expected
   return 0;
}

int test_controller() {

	if(vbe_get_ctrl_info() != 0){
    printf("video_test_controller::Error in getting controller info!\n");
    return -1;
  }

  //Everything went as expected
  return 0;
}
