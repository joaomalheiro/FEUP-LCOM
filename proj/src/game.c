#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/driver.h>
#include "game.h"
#include "video_gr.h"
#include "keyboard.h"
#include "mouse.h"
#include "timer.h"
#include "video_utils.h"
#include "robinix.h"
#include "rtc.h"
#include "uart.h"
#include "font.h"
#include "scoremanager.h"

//Currently used video mode
#define GAME_VIDEO_MODE 0x117
#define FRAME_RATE 60

//Global private variable to point to the robinix "object" (holds information about the game state, basically is a game object)
static Robinix * rob;

Robinix * get_rob() {
  return rob;
}

int play_game() {
  //Setting the video mode
  if(vg_init(GAME_VIDEO_MODE) == NULL){
    printf("video_test_play::Error initializing video mode!\n");
    return -1;
  }

  //Creating the "game state object"
  rob = create_robinix();

  //Error creating game object
  if(rob == NULL){
    printf("video_test_play::Error creating game object!\n");
    return -2;
  }

  //Subscribing to the peripherals
  if(subscribe_peripherals(rob) != SUBS_OK){
    printf("video_test_play::Error subscribing peripherals!");
    return -3;
  }

  //Interrupt loop variables
  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;

  //Interrupt loop
  while(get_game_state(rob) != EXIT_GAME) {

   //Receiving messages from the DD
   if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
     printf("video_test_play::Error in driver_receive() with: %d", driver_receive_errorlevel);
     continue;
   }

   //Verifying if the message received is a notification
   //(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
   if (is_ipc_notify(ipcStatus)) {
     switch (_ENDPOINT_P(msg.m_source)){
       //In the case that the notification is sent by the hardware
       case HARDWARE:
       //Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & rob->keyboard_irq_bitmask) {
           //Executing keyboard IH
           if(keyboard_IH() != 0){
              printf("video_test_play::Error in keyboard IH\n");
              return -5;
           }
         }

         //Verifying if the interrupt received is the keyboard interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & rob->mouse_irq_bitmask) {
           //Executing mouse IH
           if(mouse_IH() != 0) {
             printf("video_test_play::Error in mouse IH\n");
             return -6;
           }
         }
         //Verifying if the interrupt received is the timer interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & rob->timer_irq_bitmask) {
           timer_IH();
         }

         //Verifying if the interrupt received is the rtc interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & rob->rtc_irq_bitmask) {
           //Executing RTC IH
           if(rtc_IH() != 0) {
             printf("video_test_play::Error in RTC IH\n");
             return -7;
           }
         }

         //Verifying if the interrupt received is an UART interrupt, by using the irq bitmask previously created
         if(msg.NOTIFY_ARG & rob->uart_irq_bitmask) {
           //Executing UART IH
           if(uart_IH() != 0) {
              printf("video_test_play::Error in UART IH\n");
              return -4;
           }
         }
         break;
         //No other type of notification is expected, so we do nothing
       default:
       break;
       }
     }
   }

  //Unsubscribing from the peripherals
  if(unsubscribe_peripherals(rob) != UNSUBS_OK){
    printf("video_test_play::Error unsubscribing peripherals!");
    return -8;
  }

  //Destroying game object
  //Passing address so the pointer can be set to null upon deallocation
  destroy_robinix(&rob);

  //Exiting video mode
  if(vg_exit() != 0){
    printf("video_test_play::Error exiting video mode\n");
    return -9;
  }

  //Everything went as expected
  return 0;
}

int test_uart_tx(char * string) {
  if(string == NULL || strlen(string) == 0) {
    printf("test_uart_tx::Invalid input string!\n");
    return 1;
  }

  int uart_irq_bitmask = uart_subscribe_int();

  if(uart_irq_bitmask == -1 || uart_irq_bitmask == -2 || uart_irq_bitmask == -3 || uart_irq_bitmask == -4 || uart_irq_bitmask == -5) {
    printf("test_uart_tx::Error subscribing to UART interrupts\n");
    return -1;
  }

  //8 bpc, 1 stop bit, parity
  //1 is odd parity, 2 is even
  if(uart_set_conf(8, 1, 1) != 0) {
    printf("test_uart_tx::Error setting UART conf\n");
    return -2;
  }

  if(uart_set_rate(1200) != 0) {
    printf("test_uart_tx::Error setting UART rate\n");
    return -3;
  }

  printf("TX:After setting conf\n");
  uart_get_conf();
  uart_get_rate();

  //Requesting sending of the message passed
  uart_send_string(string);

  //Interrupt loop
  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;

  while(true) {
    //Receiving messages from the DD
    if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
      printf("test_uart_rx::Error in driver_receive() with: %d", driver_receive_errorlevel);
      continue;
    }

    //Verifying if the message received is a notification
    //(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
    if (is_ipc_notify(ipcStatus)) {
      switch (_ENDPOINT_P(msg.m_source)){
        //In the case that the notification is sent by the hardware
        case HARDWARE:
          //Verifying if the interrupt received is an UART interrupt, by using the irq bitmask previously created
          if(msg.NOTIFY_ARG & uart_irq_bitmask) {
            //Executing UART IH
            if(uart_IH() != 0){
               printf("test_uart_tx::Error in UART IH\n");
               return -4;
            }
          }
          break;
          //No other type of notification is expected, so we do nothing
        default:
          break;
      }
    }
  }

  if(uart_unsubscribe_int() != 0) {
    printf("test_uart_tx::Error unsubscribing from UART interrupts\n");
    return -5;
  }

  printf("Everything went as expected\n");
  return 0;
}

int test_uart_rx() {
  int uart_irq_bitmask = uart_subscribe_int();

  if(uart_irq_bitmask == -1 || uart_irq_bitmask == -2 || uart_irq_bitmask == -3 || uart_irq_bitmask == -4 || uart_irq_bitmask == -5) {
    printf("test_uart_rx::Error subscribing to UART interrupts\n");
    return -1;
  }

  //8 bpc, 1 stop bit, parity
  //1 is odd parity, 2 is even
  if(uart_set_conf(8, 1, 1) != 0) {
    printf("test_uart_rx::Error setting UART conf\n");
    return -2;
  }

  if(uart_set_rate(1200) != 0) {
    printf("test_uart_rx::Error setting UART rate\n");
    return -3;
  }

  printf("RX:After setting conf\n");
  uart_get_conf();
  uart_get_rate();

  int driver_receive_errorlevel;
  int ipcStatus;
  message msg;

  while(true) {
    //Receiving messages from the DD
    if ((driver_receive_errorlevel = driver_receive(ANY, &msg, &ipcStatus)) != 0) {
      printf("test_uart_rx::Error in driver_receive() with: %d", driver_receive_errorlevel);
      continue;
    }

    //Verifying if the message received is a notification
    //(If it is a standard message nothing is supposed to be done, as no standard messages are expected)
    if (is_ipc_notify(ipcStatus)) {
      switch (_ENDPOINT_P(msg.m_source)){
        //In the case that the notification is sent by the hardware
        case HARDWARE:
          //Verifying if the interrupt received is an UART interrupt, by using the irq bitmask previously created
          if(msg.NOTIFY_ARG & uart_irq_bitmask) {
            //Executing UART IH
            if(uart_IH() != 0){
               printf("test_uart_rx::Error in UART IH\n");
               return -4;
            }
          }
          break;
          //No other type of notification is expected, so we do nothing
        default:
          break;
      }
    }
  }

  if(uart_unsubscribe_int() != 0) {
    printf("test_uart_rx::Error unsubscribing from UART interrupts\n");
    return -5;
  }

  printf("Everything went as expected\n");
  return 0;
}
