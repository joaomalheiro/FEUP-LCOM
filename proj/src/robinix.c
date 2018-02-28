#include "robinix.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

//Peripherals
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "uart.h"
//For mouse commands
#include "i8042.h"

#include "level.h"
#include "font.h"
#include "menumanager.h"
#include "scoremanager.h"
#include "gamestats.h"
//Temporary probably:
#include "video_gr.h"
#include "video_utils.h"

////Helpful private functions

//Receives x and y variables and width and height and makes sure that they are inside the screen
static void limit_xy_inside_screen(long * x, long * y, int width, int height) {
  //NOTE: x and y are centered in the top left corner, thus the verifications are done like so

  if(*x <= 0) {
    *x = 0;
  } else if(*x + width >= getHorResolution()) {
    *x = getHorResolution() - width;
  }

  if(*y <= 0) {
    *y = 0;
  } else if(*y + height >= getVerResolution()) {
    *y = getVerResolution() - height;
  }
}

////Public functions

Robinix * create_robinix() {

  //Allocating space for the game struct
  Robinix * rob_ptr = malloc(sizeof *rob_ptr);
  //Checking if the memory was allocated correctly
  if(rob_ptr == NULL) {
    return NULL;
  }

  //Setting default values to ensure there are no problems (and game starts in correct initial state)
  rob_ptr->n_events_to_process = 0;
  rob_ptr->timer_ticks_playing = 0;
  rob_ptr->mp_msg_delay_ticks = 0;
  rob_ptr->mp_syncing_ticks = 0;
  rob_ptr->n_ticks_synced = 0;
  rob_ptr->tick_decided = 0;
  rob_ptr->comm_state = COMM_WAITING_TO_PING;

  //(Using a compound literal for initialization)
  //Also doing key state initialization here
  rob_ptr->currstate = (State) {.mouseX = MOUSE_START_X, .mouseY = MOUSE_START_Y, .state = MENU,
                                   .w_pressed = false, .a_pressed = false, .s_pressed = false, .d_pressed = false};


  //Ensuring everything pointer or pointer-like is NULL initialized to prevent problems
  rob_ptr->event_buffer = NULL;
  rob_ptr->snapshot_buffer = NULL;
  rob_ptr->level = NULL;
  rob_ptr->game_stats = NULL;

  //Loading the bitmap of the pause menu into memory
  rob_ptr->pause_menu_bmp = loadBitmap("/home/Robinix/res/img/other/pause_menu.bmp");

  //If the pause menu bmp was not correctly allocated, destroy the game object and return NULL
  if(rob_ptr->pause_menu_bmp == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  //Loading the menu manager
  rob_ptr->menu_man = create_menumanager();

  //If the menu manager was not correctly created, destroy the game object and return NULL
  if(rob_ptr->menu_man == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  //Creating the score manager
  rob_ptr->score_man = create_scoremanager();

  //If the score manager was not correctly created, destroy the game object and return NULL
  if(rob_ptr->score_man == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  //Loading the bitmap of the mouse pointer into memory (for use in menus, in levels Level object takes care of rendering the mouse due to mouse overs)
  rob_ptr->mouse_bmp = loadBitmap("/home/Robinix/res/img/mouse/mouse_menu.bmp");

  //If the mouse pointer bmp was not correctly allocated, free the already allocated memory and quit (returning NULL)
  if(rob_ptr->mouse_bmp == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  //Loading the bitmap of the win screen into memory
  rob_ptr->win_screen_bmp = loadBitmap("/home/Robinix/res/img/other/win_screen.bmp");

  //If the win screen bmp was not correctly allocated, destroy the object and return NULL
  if(rob_ptr->win_screen_bmp == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  //Loading the sprite of the lose screen (Yes I know it's a lot of bitmaps - exactly 37, please don't kill me - I had 74 previously :D)
  char * lose_screen_sprite_paths[] = {"/home/Robinix/res/img/other/lose_screen/lose_screen_1.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_2.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_3.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_4.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_5.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_6.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_7.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_8.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_9.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_10.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_11.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_12.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_13.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_14.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_15.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_16.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_17.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_18.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_19.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_20.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_21.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_22.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_23.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_24.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_25.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_26.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_27.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_28.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_29.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_30.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_31.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_32.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_33.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_34.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_35.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_36.bmp", "/home/Robinix/res/img/other/lose_screen/lose_screen_37.bmp"};

  rob_ptr->lose_screen_sprite = create_sprite(lose_screen_sprite_paths, 37, 16);

  if(rob_ptr->lose_screen_sprite == NULL) {
    destroy_robinix(&rob_ptr);
    return NULL;
  }

  return rob_ptr;
}

int subscribe_peripherals(Robinix * rob) {

  rob->timer_irq_bitmask = timer_subscribe_int();

  if (rob->timer_irq_bitmask == -1 || rob->timer_irq_bitmask == -2) {
    return TIMER_SUB_ERROR;
  }

  rob->keyboard_irq_bitmask = keyboard_subscribe_int();

  if (rob->keyboard_irq_bitmask == -1 || rob->keyboard_irq_bitmask == -2) {
    return KB_SUB_ERROR;
  }

  //Subscribing first to be able to send commands
  rob->mouse_irq_bitmask = mouse_subscribe_int();

  if(rob->mouse_irq_bitmask == -1 || rob->mouse_irq_bitmask == -2) {
    return MOUSE_SUB_ERROR;
  }

  //Setting the relevant modes for the mouse
  //Enabling stream mode since it is disabled in text mode by default
	if(mouse_write_byte(MOUSE_SET_STREAMMODE) != 0){
		return MOUSE_SUB_ERROR;
	}

	//Enabling data reporting
	if(mouse_write_byte(MOUSE_ENABLE) != 0){
		return MOUSE_SUB_ERROR;
	}

  rob->rtc_irq_bitmask = rtc_subscribe_int();

  if(rob->rtc_irq_bitmask == -1 || rob->rtc_irq_bitmask == -2 || rob->rtc_irq_bitmask == -3) {
    return RTC_SUB_ERROR;
  }

  rob->uart_irq_bitmask = uart_subscribe_int();

  if(rob->uart_irq_bitmask == -1 || rob->uart_irq_bitmask == -2 || rob->uart_irq_bitmask == -3 || rob->uart_irq_bitmask == -4 || rob->uart_irq_bitmask == -5) {
    return UART_SUB_ERROR;
  }

  //Setting the desired configurations for the UART
  //8 bpc, 1 stop bit, parity
  //1 is odd parity, 2 is even
  if(uart_set_conf(8, 1, 1) != 0) {
    return UART_SUB_ERROR;
  }

  if(uart_set_rate(1200) != 0) {
    return UART_SUB_ERROR;
  }

  //Add other subscribes here

  return SUBS_OK;
}

int unsubscribe_peripherals(Robinix * rob) {

  if(timer_unsubscribe_int() != 0){
    return TIMER_UNSUB_ERROR;
  }

  if(keyboard_unsubscribe_int() != 0){
    return KB_UNSUB_ERROR;
  }

  //Disabling data reporting
  if(mouse_write_byte(MOUSE_DISABLE) != 0){
    return MOUSE_UNSUB_ERROR;
  }

  if(mouse_unsubscribe_int() != 0){
    return MOUSE_UNSUB_ERROR;
  }

  if(rtc_unsubscribe_int() != 0) {
    return RTC_UNSUB_ERROR;
  }

  if(uart_unsubscribe_int() != 0) {
    return UART_UNSUB_ERROR;
  }

  //Clearing the outbuffer so that future keyboard and mouse interrupt subscription work correctly
  keyboard_clear_outbuf();

  return UNSUBS_OK;
}

//Clears the game snapshot taken
static void clear_game_snapshot(Robinix * rob) {
  if(rob->snapshot_buffer == NULL) {
    return;
  }

  free(rob->snapshot_buffer);
  rob->snapshot_buffer = NULL;
}

void destroy_robinix(Robinix ** rob) {
  //If the pointer is already pointing to NULL, we do nothing
  if(*rob == NULL){
    return;
  }

  //Clearing the event buffer (Calls destroy_event for each event and deallocates the array itself)
  clear_event_buffer(*rob);

  //Clearing the mouse bitmap stored
  deleteBitmap((*rob)->mouse_bmp);
  //Destroying menu manager object
  destroy_menumanager(&((*rob)->menu_man));
  //Clearing the pause menu bitmap stored
  deleteBitmap((*rob)->pause_menu_bmp);
  //Clearing the win screen bitmap stored
  deleteBitmap((*rob)->win_screen_bmp);
  //Destroying the lose screen sprite
  destroy_sprite(&((*rob)->lose_screen_sprite));
  //Destroying score manager object
  destroy_scoremanager(&((*rob)->score_man));
  //Destroying game stats object if allocated
  destroy_gamestats(&((*rob)->game_stats));
  //Destroying level object if allocated
  destroy_level(&((*rob)->level));

  //Clearing snapshot buffer if still allocated
  clear_game_snapshot(*rob);

  //NOTE: Don't forget to update with more deallocations if there are any, eventually

  //Finally, deallocating Robinix struct
  free(*rob);
  //Setting the pointer to the Robinix struct to NULL so we can know that the object has been deallocated
  //(This is the reason for using a Robinix ** and not a simple Robinix * like in all the other functions)
  *rob = NULL;
}

void add_event_to_buffer(Robinix * rob, Event * evt) {
  ///Malloc'ing when the array is full is not necessary since:
  ///"[When calling realloc with a pointer called ptr,] if ptr is NULL, the behavior is the same as calling malloc(new_size)."

  //Checking if the passsed Event is not NULL. If it is, do nothing
  if(evt == NULL){
    return;
  }

  //Attempting to reallocate the array to a different size (1 more element)
  Event ** temp_event_buffer = realloc(rob->event_buffer, (rob->n_events_to_process + 1) * sizeof *(rob->event_buffer));

  if(temp_event_buffer){
    //If the reallocated pointer is valid
    //Changing the buffer to be the newly reallocated one (previous one is invalidated, so there is no need to call free on the old event buffer)
    rob->event_buffer = temp_event_buffer;
    //Updating the number of events to process
    rob->n_events_to_process++;
    //Adding the passed event to the end of the buffer
    rob->event_buffer[rob->n_events_to_process - 1] = evt;
  } else {
    //Error, could not reallocate to a bigger buffer
    printf("ERROR in add_event_to_buffer, event buffer could not be reallocated to a bigger size!!!\n");
    //Freeing the event since it could not be added to the buffer...
    free(evt);
    evt = NULL;
    //Note: temp_event_buffer does not need to be freed since realloc returns NULL on failure
    return;
  }

}

Event * create_event(event_enum evt_type, int mouse_move_x, int mouse_move_y, char pressed_key, char * remote_msg) {
  Event * evt = malloc(sizeof *evt);

  if(evt == NULL){
    return NULL;
  }

  //Assing values passed to the allocated Event
  evt->evt_type = evt_type;
  evt->mouse_move_x = mouse_move_x;
  evt->mouse_move_y = mouse_move_y;
  evt->pressed_key = pressed_key;
  evt->remote_msg = remote_msg;

  /*
  //TEMP orary
  if(evt->evt_type == RECEIVED_REMOTE_MESSAGE) {
    printf("DBG: Received UART event with content: \"%s\"\n", evt->remote_msg);
  }
  */

  //Returning the created Event
  return evt;
}

void destroy_event(Event ** evt) {
  if(*evt == NULL) {
    return;
  }

  //Also deallocating the received message string
  //If the event is not one from remote communication this is NULL and thus there is no problem with the free as well
  free((*evt)->remote_msg);

  //(Don't forget that free also checks for NULL, so there is no problem if evt is already NULL)
  free(*evt);
  *evt = NULL;
}

void clear_event_buffer(Robinix * rob) {
  //If there are events to process stored, destroy them
  if(rob->n_events_to_process > 0){
    int i;
    for(i = 0; i < rob->n_events_to_process; i++){
      //Destroying an event if it is allocated
      destroy_event(&(rob->event_buffer[i]));
    }

    //Updating the number of events to process variable
    rob->n_events_to_process = 0;

    //Deallocating space in memory for the array (** was used so this must be done)
    free(rob->event_buffer);
    rob->event_buffer = NULL;
  }

}

int game_load_level(Robinix * rob, int level, bool is_mp) {
  //Just to be super safe, resetting the timer ticks
  rob->timer_ticks_playing = 0;
  //Stopping key stuck from any previous state
  rob->currstate.w_pressed = false;
  rob->currstate.a_pressed = false;
  rob->currstate.s_pressed = false;
  rob->currstate.d_pressed = false;

  //Resetting auxiliar variables for score just in case
  free(rob->current_date_string);
  rob->current_date_string = NULL;
  free(rob->time_taken);
  rob->time_taken = NULL;
  //To reset a char [] we can just write a null terminator to the first index
  rob->player_name[0] = '\0';
  rob->player_score = 0;

  //Also allocating our game stats manager
  //(And deallocating it if it is allocated, before trying to reallocate)
  if(rob->game_stats != NULL) {
    destroy_gamestats(&(rob->game_stats));
  }

  rob->game_stats = create_gamestats();

  if(rob->game_stats == NULL) {
    printf("DBG: Failed to create game_stats\n");
    return -1;
  }

  //If a level was previously allocated, destroy it before trying to reallocate
  if(rob->level != NULL) {
    destroy_level(&(rob->level));
  }

  rob->level = create_level(level, is_mp);

  if(rob->level == NULL) {
    printf("DBG: Failed to create level %d, mp flag: %d\n", level, is_mp);
    destroy_gamestats(&(rob->game_stats));
    return -2;
  } else {
    return 0;
  }
}

state_enum get_game_state(Robinix * rob) {
  return rob->currstate.state;
}

//Gets a snapshot of the current game, and makes it black and white for better visual effect
static void snapshot_game(Robinix * rob) {
  //If a snapshot had been taken, clear it before taking a new one
  if(rob->snapshot_buffer != NULL) {
    clear_game_snapshot(rob);
  }

  rob->snapshot_buffer = snapshot_video_mem();
  if(rob->snapshot_buffer == NULL) {
    //If snapshot unsuccessful, do nothing
    return;
  }

  //Making the snapshot black and white (video_mem was the buffer that was snapshotted so it is the size of the Vram)
  make_bw(rob->snapshot_buffer, getVramSize());

  //Fade slightly, for better look (pause screen was too harsh)
  change_gamma(rob->snapshot_buffer, getVramSize(), 0.4);

  //The screen was now snapshotted and that snapshot made B&W! Done!
}

//For use in menus (pause, lose, win and main menu)
static void game_draw_mouse(Robinix * rob) {
  drawBitmap(rob->mouse_bmp, rob->currstate.mouseX, rob->currstate.mouseY);
}

//Draws the buffer snapshot
static void game_draw_snapshot_buffer(Robinix * rob) {
  redraw_buffer(rob->snapshot_buffer);
}

static void game_draw_pause_menu(Robinix * rob) {
  drawBitmap(rob->pause_menu_bmp, 0, 0);
}

static void draw_menu_highscores(Robinix * rob) {
  display_highscores(rob->score_man, 20, 240, "kenneypixel-38", 60);
}

static void draw_date_in_menu() {
  Date_obj temp = get_current_date();
  char * text = date_to_string(&temp);
  //768 - 22 = 746 +- mais uns pozinhos, 740
  string_to_screen(text, "monofonto-22", 10, 740);
  //Text string no longer necessary (Date_obj does not need to be free'd since it is stack allocated only)
  free(text);
}

static void draw_game_stats(Robinix * rob) {
  gamestats_draw(rob->game_stats);
}

static void game_draw_lose_sprite(Robinix * rob) {
  draw_sprite(rob->lose_screen_sprite, 0, 0);
}

static void game_draw_win_screen_background(Robinix * rob) {
  game_draw_snapshot_buffer(rob);
  drawBitmap(rob->win_screen_bmp, 0, 0);
}

static void game_draw_win_screen_text(Robinix * rob) {
  //Drawing score (420, 393)
  //Score is unsigned int, need to convert to string
  //The maximum value of a 16 bit unsigned int is 65k which is 5 characters (+1 for \0)
  //Just in case, considering a 32bit unsigned int, whose max value is around 4 million - 10 characters (+1 for \0)
  char temp_buf[11];
  sprintf(temp_buf, "%u", rob->player_score);
  string_to_screen(temp_buf, "monofonto-22", 420, 390);
  //Drawing time taken (725, 393)
  string_to_screen(rob->time_taken, "monofonto-22", 725, 390);
  //Drawing currently inputted name (310, 503)
  string_to_screen(rob->player_name, "monofonto-22", 310, 500);

  if(rob->is_new_highscore) {
    //In case the player had a higher score than the top scoring highscore then display "New Highscore" message on screen
    string_to_screen("NEW HIGHSCORE", "monofonto-22", 354, 220);
  }

  //No need to free temp_buf because it was stack allocated and not heap allocated (no malloc, no free)
}

void game_draw(Robinix * rob) {
  switch(rob->currstate.state) {
    //While playing all drawing is handled by the Level object (except game stats)
    case PLAYING_SP:
      draw_level(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
      draw_game_stats(rob);
      break;
    case PAUSED_SP:
      game_draw_snapshot_buffer(rob);
      game_draw_pause_menu(rob);
      game_draw_mouse(rob);
      break;
    case LOSE_SP:
      game_draw_lose_sprite(rob);
      game_draw_mouse(rob);
      break;
    case MENU:
      //Menu manager handles most of the drawing, except for the mouse and highscores
      if(draw_menumanager(rob->menu_man) == 1) {
        //Returning 1 is the way that the menumanger has to request highscores drawing
        draw_menu_highscores(rob);
      }
      draw_date_in_menu();
      game_draw_mouse(rob);
      break;
    case SCORE_SUBMIT:
      game_draw_win_screen_background(rob);
      game_draw_win_screen_text(rob); //Score, time taken, current player input name
      game_draw_mouse(rob);
      break;
    case SEARCHING_MP:
      game_draw_snapshot_buffer(rob);
      string_to_screen("Waiting for other player", "monofonto-22", 270, 240);
      game_draw_mouse(rob);
      break;
    case SYNCING_MP:
      game_draw_snapshot_buffer(rob);
      if(rob->isPlayer1) {
        string_to_screen("Syncing with player 2...", "monofonto-22", 280, 240);
      } else {
        string_to_screen("Syncing with player 1...", "monofonto-22", 280, 240);
      }
      game_draw_mouse(rob);
      break;
    case WAITING_MP:
      //Using snapshot for better look
      game_draw_snapshot_buffer(rob);
      string_to_screen("Get ready", "monofonto-22", 335, 350);
      //Getting the delta in ticks and dividing by 60 to convert to seconds
      unsigned int seconds_to_start = (rob->tick_decided - rob->mp_syncing_ticks) / 60;
      char temp_str[20];
      sprintf(temp_str, "Starting in %u", seconds_to_start);
      string_to_screen(temp_str, "monofonto-22", 300, 380);
      game_draw_mouse(rob);
      break;
    case PLAYING_MP:
      draw_level(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
      draw_game_stats(rob);
      //game_draw_mouse(rob); //Level already draws mouse
      break;
    case LOSE_MP:
      game_draw_lose_sprite(rob);
      game_draw_mouse(rob);
      break;
    default:
      break;
  }
}

static void game_update_playing_sp(Robinix * rob) {
  //While playing updates are handled by Level object
  update_level(rob->level, rob);
  //Except for ticking timer interrupts and stats time, which is done here
  rob->timer_ticks_playing++;
  if(rob->timer_ticks_playing % 60 == 0) {
    gamestats_tick_time(rob->game_stats);
  }
}

static void game_update_searching_mp(Robinix * rob) {
  rob->mp_msg_delay_ticks++;
  if(rob->comm_state == COMM_WAITING_TO_PING && rob->mp_msg_delay_ticks >= COMM_DELAY_UNTIL_PINGING) {
    rob->comm_state = COMM_PINGING;
    printf("DBG: Started pinging at tick %u\n", rob->mp_msg_delay_ticks);
  }

  if(rob->mp_msg_delay_ticks % COMM_MSG_RETRY_TICKDELAY == 0) {
    switch(rob->comm_state) {
      case COMM_WAITING_TO_PING:
        //Still waiting until sending the first ping, do nothing
        break;
      case COMM_PINGING:
        printf("DBG: Sending searching msg at tick %u\n", rob->mp_msg_delay_ticks);
        uart_send_string(COMM_SEARCHING_STR);
        break;
      case COMM_REPLYING:
        printf("DBG: Sending reply msg at tick %u\n", rob->mp_msg_delay_ticks);
        uart_send_string(COMM_REPLY_STR);
        break;
      case COMM_ACKING:
        printf("DBG: Sending ACKs at tick %u\n", rob->mp_msg_delay_ticks);
        uart_send_string(COMM_ACKNOWLEDGE_REPLY);
        break;
      default:
        printf("game_update_searching_mp::Erroneous state, comm_state: %d\n", rob->comm_state);
        break;
    }
  }
}

static void game_update_syncing_mp(Robinix * rob) {
  //Player 1 is host
  if(rob->isPlayer1) {
    if(rob->mp_msg_delay_ticks % COMM_TICK_SYNC_DELAY == 0) {
      //A very large buffer can be used with no negative effects since uart_send_string copies the string until strlen so no unnecessary space is used
      //The buffer is also stack allocated so no large block of memory is kept in use indefinitely
      char sync_msg[45];
      sprintf(sync_msg, "%s%llu", COMM_SYNC_PREFIX, rob->mp_syncing_ticks);
      //printf("DBG: Sending syncing msg: %s\n", sync_msg);
      uart_send_string(sync_msg);
    }
    rob->mp_msg_delay_ticks++;
  }
  //Both must update ticks for every timer interrupt (update call)
  rob->mp_syncing_ticks++;
  /*
  //TEMP for testing if are synced manually
  if(rob->mp_syncing_ticks % 6 == 0) {
    //TEMP Debug
    //Since printf does not accept %llu, we have to use sprintf before to be able to print the value on screen
    char buf[30];
    sprintf(buf, "%llu", rob->mp_syncing_ticks);
    printf("DBG: Current timer tick: %s\n", buf);
  }
  */
}

static void game_update_waiting_mp(Robinix * rob) {
  rob->mp_syncing_ticks++;

  if(rob->mp_syncing_ticks % 10 == 0) {
    //TEMP Debug
    //Since printf does not accept %llu, we have to use sprintf before to be able to print the value on screen
    //char buf[30];
    //sprintf(buf, "%llu", rob->mp_syncing_ticks);
    //printf("DBG: Waiting, current timer tick: %s\n", buf);
  }

  if(rob->mp_syncing_ticks == rob->tick_decided) {
    //Entering multiplayer play state, correct objects were already loaded when entering waiting to prevent load stutter
    //Clear the snapshot that was being used as background
    clear_game_snapshot(rob);
    rob->currstate.state = PLAYING_MP;
  }
}

static void game_update_playing_mp(Robinix * rob) {
  rob->mp_syncing_ticks++;
  //Ticking time for displaying
  if(rob->mp_syncing_ticks % 60 == 0) {
    gamestats_tick_time(rob->game_stats);
  }
  //All other updating is done by level object (moving guards, etc)
  update_level(rob->level, rob);
}

void game_update(Robinix * rob) {
  switch (rob->currstate.state) {
    case PLAYING_SP:
      game_update_playing_sp(rob);
      break;
    case PAUSED_SP:
      //Do not need to update anything when paused, since events are doing the processing anyway
      //Update would just update player and guard positions, which is not desired when paused (everything is stopped)
      break;
    case LOSE_SP:
      //Do not need to update anything after losing, since events are doing the processing anyway
      //Update would just update player and guard positions, which is not desired after losing (everything is stopped)
      break;
    case SEARCHING_MP:
      //Using update to count ticks between which to send the "searching for player" "beacon" messages
      game_update_searching_mp(rob);
      break;
    case SYNCING_MP:
      //Using update to count ticks and send tick messages for syncing
      game_update_syncing_mp(rob);
      break;
    case WAITING_MP:
      //Must update ticks to know when to start the game
      game_update_waiting_mp(rob);
      break;
    case PLAYING_MP:
      game_update_playing_mp(rob);
      break;
    default:
      break;
  }
}

static void game_process_event_menu(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_UP:
      //Was only used for escape exiting the game for debug purposes, now does nothing
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;

      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);

      //Updating the mouse_over menu buttons
      menumanager_update_mouse_over(rob->menu_man, rob->currstate.mouseX, rob->currstate.mouseY);
      break;
    case MOUSE_LB_DOWN:
      menumanager_handle_button_click(rob->menu_man, rob);
      break;
    case CLICKED_PLAY_MULTI:
      //Enter searching for players mode, to check for player 2
      rob->currstate.state = SEARCHING_MP;
      //Resetting the number of ticks elapsed during searching
      rob->mp_msg_delay_ticks = 0;
      //Resetting communications state
      rob->comm_state = COMM_WAITING_TO_PING;
      //Using snapshot as background so take it here
      snapshot_game(rob);
      break;
    case CLICKED_PLAY_LVL_1:
      //Not entering the game in case of failure, better to not enter than to crash
      if(game_load_level(rob, 1, false) != 0) {
        printf("DBG: Error loading level 1\n");
      } else {
        rob->currstate.state = PLAYING_SP;
      }
      break;
    case CLICKED_PLAY_LVL_2:
      //Not entering the game in case of failure, better to not enter than to crash
      if(game_load_level(rob, 2, false) != 0) {
        printf("DBG: Error loading level 2\n");
      } else {
        rob->currstate.state = PLAYING_SP;
      }
      break;
    case CLICKED_EXIT_GAME:
      //Before exiting the game save the scores to a file (can't be done in destroy_robinix since that would be called on errors as well)
      write_scores_to_scores_file(rob->score_man);
      rob->currstate.state = EXIT_GAME;
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_event_playing_sp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_DOWN:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = true;
          break;
        case 'a':
          rob->currstate.a_pressed = true;
          break;
        case 's':
          rob->currstate.s_pressed = true;
          break;
        case 'd':
          rob->currstate.d_pressed = true;
          break;
        default:
          break;
      }
      break;
    case KEY_UP:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = false;
          break;
        case 'a':
          rob->currstate.a_pressed = false;
          break;
        case 's':
          rob->currstate.s_pressed = false;
          break;
        case 'd':
          rob->currstate.d_pressed = false;
          break;
        case '!':
          //Pressing escape pauses the game
          rob->currstate.state = PAUSED_SP;
          //Pausing the game requires a snapshot to be taken to use as a background
          snapshot_game(rob);
          break;
        default:
          break;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Updating mouse_overs in level
      level_update_mouse_over(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
      break;
    case MOUSE_LB_DOWN:
      level_handle_mouse_click(rob->level);
      break;
    case PLAYER_GOT_COIN:
      gamestats_tick_coins(rob->game_stats);
      break;
    case PLAYER_COLLIDE_WITH_GUARD:
      rob->currstate.state = LOSE_SP;
      //Upon losing, level and gamestats are destroyed to save memory
      destroy_level(&(rob->level));
      destroy_gamestats(&(rob->game_stats));
      break;
    case PLAYER_COLLIDE_WITH_EXIT:
      //On colliding with exit (only possible after picking up the treasure before) go into score submission mode
      //First, we calculate player score to have it stored for display later on
      rob->player_score = gamestats_calculate_score(rob->game_stats);
      //Also saving time taken for displaying
      rob->time_taken = gamestats_get_time_taken(rob->game_stats);
      //Checking if the score is a new highscore
      if(rob->player_score > scoremanager_get_highest_score(rob->score_man)) {
        rob->is_new_highscore = true;
      } else {
        rob->is_new_highscore = false;
      }
      rob->currstate.state = SCORE_SUBMIT;
      //Upon winning, level is destroyed to save memory
      destroy_level(&(rob->level));
      //Game Stats as well since we already got what we wanted from it (time taken as hh:mm:ss string and calculated score)
      destroy_gamestats(&(rob->game_stats));
      //Winning the game uses a snapshot as a background
      snapshot_game(rob);
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_event_paused(Robinix * rob, Event * evt) {
  //NOTE: Movement events (wasd) are still being processed all the same so that the player isn't stuck stopped or moving (and has to repress keys)
  switch (evt->evt_type) {
    case KEY_DOWN:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = true;
          break;
        case 'a':
          rob->currstate.a_pressed = true;
          break;
        case 's':
          rob->currstate.s_pressed = true;
          break;
        case 'd':
          rob->currstate.d_pressed = true;
          break;
        default:
          break;
      }
      break;
    case KEY_UP:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = false;
          break;
        case 'a':
          rob->currstate.a_pressed = false;
          break;
        case 's':
          rob->currstate.s_pressed = false;
          break;
        case 'd':
          rob->currstate.d_pressed = false;
          break;
        case '!':
          //Pressing escape again resumes the game
          rob->currstate.state = PLAYING_SP;
          //Snapshot is no longer necessary, we can therefore clear the game snapshot
          clear_game_snapshot(rob);
          break;
        case 'm':
          //Pressing m goes back to the main menu
          //Upon going back to the main menu, level is destroyed to save memory
          destroy_level(&(rob->level));
          //Resetting menu before going there (Going back to main menu, etc)
          reset_menumanager(rob->menu_man);
          rob->currstate.state = MENU;
          break;
        default:
          break;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Checking if mouse is leaving screen and stopping it from doing so
      //limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      //This is now done inside level object while playing
      break;
    case MOUSE_LB_DOWN:
      break;
    case MOUSE_RB_DOWN:
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_event_lose_sp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_DOWN:
      break;
    case KEY_UP:
      switch (evt->pressed_key) {
        case '!':
          //Resetting menu before going there (Going back to main menu, etc)
          reset_menumanager(rob->menu_man);
          rob->currstate.state = MENU;
          break;
        default:
          break;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_score_submit(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_DOWN:
      switch (evt->pressed_key) {
        case '!':
          //Escape is not being used but it is so that it is captured here and not in the default block
          break;
        case ' ':
          //Because of limitations on the file writing and reading algorithm, it is not possible to have a name with spaces
          //Thus the case is captured here to not be treated in the default block
          break;
        case 'E':
          //Enter was pressed, user desires to submit score
          //Just to not be weird and possibly cause problems check if the name as one character at least, before submitting
          if(strlen(rob->player_name) > 0) {
            Date_obj temp_date = get_current_date();
            rob->current_date_string = date_to_date_string(&temp_date);
            //Submitting score and going back to main menu
            add_score_to_scoremanager(rob->score_man, rob->player_score, rob->player_name, rob->current_date_string);

            //Resetting helper score variables for future use
            //temp_date is stack allocated so does not need to be free'd
            free(rob->current_date_string);
            rob->current_date_string = NULL;
            free(rob->time_taken);
            rob->time_taken = NULL;
            //To reset a char [] we can just write a null terminator to the first index
            rob->player_name[0] = '\0';
            rob->player_score = 0;

            //Freeing the snapshotted background for better memory management
            clear_game_snapshot(rob);

            //Resetting menu before going there (Going back to main menu, etc)
            reset_menumanager(rob->menu_man);
            rob->currstate.state = MENU;
          }
          break;
        case 'B':
          //Backspace was pressed, deleting last character (by replacing it with \0)
          if(strlen(rob->player_name) > 0) {
            //Have to have if case because 0-1 = -1 and index -1 is not a thing
            rob->player_name[strlen(rob->player_name) - 1] = '\0';
          }
          break;
        default:
          //The other cases are actual letters input, append it to the player name if it is under the max length
          if(strlen(rob->player_name) < PLAYER_NAME_MAX_LENGTH) {
            //Because strcat must receive a null terimanted string, we generate one temporarily in order to concatenate the symbol
            strcat(rob->player_name, (char[2]) { evt->pressed_key, '\0' });
          }
          break;
      }
      break;
    case KEY_UP:
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_searching_mp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_UP:
      if(evt->pressed_key == '!') {
        //TEMP for debug using escape to go back
        rob->currstate.state = MENU;
        clear_game_snapshot(rob);
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;

      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    case RECEIVED_REMOTE_MESSAGE:
      //If searching (or waiting to search) and other is also searching then switch to replying
      if((rob->comm_state == COMM_WAITING_TO_PING || rob->comm_state == COMM_PINGING) && strncmp(evt->remote_msg, COMM_SEARCHING_STR, strlen(COMM_SEARCHING_STR)) == 0) {
        printf("DBG: Got ping, switching to sending replies\n");
        rob->comm_state = COMM_REPLYING;
      }
      //Check for response (if was searching and got a reply, send ACK)
      if(rob->comm_state == COMM_PINGING && strncmp(evt->remote_msg, COMM_REPLY_STR, strlen(COMM_REPLY_STR)) == 0) {
        printf("DBG: Got reply, sending ACK and disabling sending pings or replies\n");
        uart_send_string(COMM_ACKNOWLEDGE_REPLY);
        rob->comm_state = COMM_ACKING;
        //If I am the ACK sender, then I am player 1 (I was searching first)
        rob->isPlayer1 = true;
        printf("DBG: I'm player 1 - Entering sync state\n");
        //Switching to syncing state
        rob->currstate.state = SYNCING_MP;
        //Reset message ticks just in case
        rob->mp_msg_delay_ticks = 0;
        //Resetting all the ticks
        rob->mp_syncing_ticks = 0;
        rob->n_ticks_synced = 0;
        rob->tick_decided = 0;
      }
      //Check for ACK to my response (if I was replying and got an ACK)
      if(rob->comm_state == COMM_REPLYING && strncmp(evt->remote_msg, COMM_ACKNOWLEDGE_REPLY, strlen(COMM_ACKNOWLEDGE_REPLY)) == 0) {
        printf("DBG: Received ACK to my reply, disabling sending messages\n");
        rob->comm_state = COMM_SYNCING;
        //If I am the ACK receiver, then I am player 2 (I was searching last)
        rob->isPlayer1 = false;
        printf("DBG: I'm player 2 - Entering sync state\n");
        //Switching to syncing state
        rob->currstate.state = SYNCING_MP;
        //Reset message ticks just in case
        rob->mp_msg_delay_ticks = 0;
        //Resetting all the ticks
        rob->mp_syncing_ticks = 0;
        rob->n_ticks_synced = 0;
        rob->tick_decided = 0;
      }
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_syncing_mp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_UP:
      if(evt->pressed_key == '!') {
        //TEMP for debug using escape to go back
        rob->currstate.state = MENU;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;

      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    case RECEIVED_REMOTE_MESSAGE:
      //Checking for old messages to know if something went wrong
      if(strncmp(evt->remote_msg, COMM_SEARCHING_STR, strlen(COMM_SEARCHING_STR)) == 0) {
        printf("Debug: Got search string while in sync!!\n");
      }
      if(strncmp(evt->remote_msg, COMM_REPLY_STR, strlen(COMM_REPLY_STR)) == 0) {
        printf("Debug: Got reply while in sync!!\n");
      }
      if(strncmp(evt->remote_msg, COMM_ACKNOWLEDGE_REPLY, strlen(COMM_ACKNOWLEDGE_REPLY)) == 0) {
        printf("Debug: Received ACK to reply while in sync!!\n");
      }

      //If not host, then sync according to received tick
      if(!(rob->isPlayer1) && strncmp(evt->remote_msg, COMM_SYNC_PREFIX, strlen(COMM_SYNC_PREFIX)) == 0) {
        //printf("DBG: Received sync message - Contents: %s\n", evt->remote_msg);

        //Parsing string into unsigned long long, the start element passed is a shift by the size of the sync message prefix
        unsigned long long temp_ull = parse_ullong(evt->remote_msg + strlen(COMM_SYNC_PREFIX), 10);
        if(temp_ull == ULLONG_MAX) {
          printf("Debug: Error in UART message to ullong conversion!\n");
        } else {
          //Valid value, verify versus current timer tick
          if(temp_ull == rob->mp_syncing_ticks) {
            printf("DBG: Ticks were synced for %d times\n", rob->n_ticks_synced);
            rob->n_ticks_synced++;
          } else {
            rob->mp_syncing_ticks = temp_ull;
            rob->n_ticks_synced = 0;
            //TEMP Since printf does not accept %llu, we have to use sprintf beforehand
            //char tempbuf[30];
            //sprintf(tempbuf, "%llu", rob->mp_syncing_ticks);
            //printf("DBG: Syncing self, tick is now %s\n", tempbuf);
          }
        }

        if(rob->n_ticks_synced >= COMM_TICK_SYNC_MIN) {
          //printf("DBG: Synced for the minimum required ticks!\n");
          //Sending "Synced!" message
          uart_send_string(COMM_SYNCED_STR);
          printf("DBG: Sent 'synced' message\n");
        }
      }

      //If not host also check if received a "starting in tick X" message
      if(!(rob->isPlayer1) && strncmp(evt->remote_msg, COMM_START_TICK_PREFIX, strlen(COMM_START_TICK_PREFIX)) == 0) {
        printf("DBG: Received a starting in tick X message\n");

        //Parsing string into unsigned long long, the start element passed is a shift by the size of the sync message prefix
        unsigned long long temp_ull = parse_ullong(evt->remote_msg + strlen(COMM_START_TICK_PREFIX), 10);
        if(temp_ull == ULLONG_MAX) {
          printf("Debug: Error in UART message to ullong conversion in start tick!\n");
        } else {
          //Valid value, set start tick
          rob->tick_decided = temp_ull;
          //TEMP Since printf does not accept %llu, we have to use sprintf beforehand
          char tick_str[30];
          sprintf(tick_str, "%llu", rob->tick_decided);
          printf("DBG: Player2, starting in tick %s\n", tick_str);
        }

        //Upon leaving state clear the game snapshot for better memory management
        clear_game_snapshot(rob);
        //Switching to waiting mode
        rob->currstate.state = WAITING_MP;
        //Switching the comm mode to "ingame none", will be used when transferring message about player being over door
        rob->comm_state = COMM_IG_NONE;
        rob->other_player_at_exit = false;
        rob->sent_at_exit = false;
        //Resetting the message delay ticks (will also be used for this)
        rob->mp_msg_delay_ticks = 0;

        //When entering waiting state, load game and take snapshot
        //I am client, I get level 2 for multiplayer
        if(game_load_level(rob, 2, true) != 0) {
          printf("DBG: Error loading mp level 2\n");
          //In case of error send abort message and go back to main menu
          uart_send_string(COMM_ABORT_STR);
          rob->currstate.state = MENU;
          //Upon leaving state clear the game snapshot for better memory management
          clear_game_snapshot(rob);
        }
        //Have to force level draw and buffer swap so that the snapshot is taken correctly... Sorry
        draw_level(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
        swap_buffers();
        snapshot_game(rob);
      }

      //If host then check for the "Synced" message by the client
      if(rob->isPlayer1 && strncmp(evt->remote_msg, COMM_SYNCED_STR, strlen(COMM_SYNCED_STR)) == 0) {
        //printf("DBG: Received synced indication, calculating and sending tick to start the game on\n");

        rob->tick_decided = rob->mp_syncing_ticks + COMM_DELTA_FOR_HANDSHAKE;
        //A very large buffer can be used with no negative effects since uart_send_string copies the string until strlen so no unnecessary space is used
        //The buffer is also stack allocated so no large block of memory is kept in use indefinitely
        char start_msg[45];
        sprintf(start_msg, "%s%llu", COMM_START_TICK_PREFIX, rob->tick_decided);
        printf("DBG: Player1, Sending starting tick msg: %s\n", start_msg);
        uart_send_string(start_msg);

        //Upon leaving state clear the game snapshot for better memory management
        clear_game_snapshot(rob);
        //Switching to waiting mode, waiting until the correct tick
        rob->currstate.state = WAITING_MP;
        //Switching the comm mode to "ingame none", will be used when transferring message about player being over door
        rob->comm_state = COMM_IG_NONE;
        rob->other_player_at_exit = false;
        rob->sent_at_exit = false;
        //Resetting the message delay ticks (will also be used for this)
        rob->mp_msg_delay_ticks = 0;

        //When entering waiting state, load game and take snapshot
        //I am host, I get level 1 for multiplayer
        if(game_load_level(rob, 1, true) != 0) {
          printf("DBG: Error loading mp level 1\n");
          //In case of error send abort message and go back to main menu
          uart_send_string(COMM_ABORT_STR);
          rob->currstate.state = MENU;
        }
        //Have to force level draw and buffer swap so that the snapshot is taken correctly... Sorry
        draw_level(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
        swap_buffers();
        snapshot_game(rob);
      }
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_waiting_mp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_UP:
      if(evt->pressed_key == '!') {
        //TEMP for debug using escape to go back
        //If exiting to menu also warn other player so that they exit as well
        uart_send_string(COMM_PLAYER_LEAVING_STR);
        rob->currstate.state = MENU;
        //If exiting, destroy all objects that can be in use and leave
        destroy_level(&(rob->level));
        destroy_gamestats(&(rob->game_stats));
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;

      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    case RECEIVED_REMOTE_MESSAGE:
      if(strncmp(evt->remote_msg, COMM_ABORT_STR, strlen(COMM_ABORT_STR)) == 0) {
        //If abort ocurred, destroy all objects that can be in use and leave as well
        destroy_level(&(rob->level));
        destroy_gamestats(&(rob->game_stats));
        rob->currstate.state = MENU;
      }
      if(strncmp(evt->remote_msg, COMM_PLAYER_LEAVING_STR, strlen(COMM_PLAYER_LEAVING_STR)) == 0) {
        //If the other player is leaving so are we
        rob->currstate.state = MENU;
        //If exiting, destroy all objects that can be in use and leave
        destroy_level(&(rob->level));
        destroy_gamestats(&(rob->game_stats));
      }
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_playing_mp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_DOWN:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = true;
          break;
        case 'a':
          rob->currstate.a_pressed = true;
          break;
        case 's':
          rob->currstate.s_pressed = true;
          break;
        case 'd':
          rob->currstate.d_pressed = true;
          break;
        default:
          break;
      }
      break;
    case KEY_UP:
      switch (evt->pressed_key) {
        case 'w':
          rob->currstate.w_pressed = false;
          break;
        case 'a':
          rob->currstate.a_pressed = false;
          break;
        case 's':
          rob->currstate.s_pressed = false;
          break;
        case 'd':
          rob->currstate.d_pressed = false;
          break;
        case '!':
          //TEMP for debug using escape to go back
          //If exiting to menu also warn other player so that they exit as well
          uart_send_string(COMM_PLAYER_LEAVING_STR);
          //Resetting menu before going there (Going back to main menu, etc)
          reset_menumanager(rob->menu_man);
          rob->currstate.state = MENU;
          //When leaving also deallocate used things
          destroy_gamestats(&(rob->game_stats));
          destroy_level(&(rob->level));
          break;
        default:
          break;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Updating mouse_overs in level (Level also checks for mouse out of bounds)
      level_update_mouse_over(rob->level, rob->currstate.mouseX, rob->currstate.mouseY);
      break;
    case MOUSE_LB_DOWN:
      level_handle_mouse_click(rob->level);
      break;
    case PLAYER_GOT_COIN:
      gamestats_tick_coins(rob->game_stats);
      //Sending message to other player so both track total coins
      uart_send_string(COMM_COIN_GOT_STR);
      break;
    case PLAYER_GOT_TREASURE:
      //Sending message so other player knows we got the treasure
      uart_send_string(COMM_TREASURE_GOT_STR);
      break;
    case PLAYER_COLLIDE_WITH_GUARD:
      //Send message so other player also knows he lost
      uart_send_string(COMM_PLAYER_LOST_STR);
      rob->currstate.state = LOSE_MP;
      //Upon losing, level and gamestats are destroyed to save memory
      destroy_level(&(rob->level));
      destroy_gamestats(&(rob->game_stats));
      break;
    case PLAYER_COLLIDE_WITH_EXIT:
      if(rob->other_player_at_exit) {
        //If other player is already at the exit, then send "we won" string and move to win state
        uart_send_string(COMM_BOTH_AT_EXIT_STR);
        //Moving to win state
        //Get stats and move to score submit screen
        //First, we calculate player score to have it stored for display later on
        rob->player_score = gamestats_calculate_score(rob->game_stats);
        //Also saving time taken for displaying
        rob->time_taken = gamestats_get_time_taken(rob->game_stats);
        //Checking if the score is a new highscore
        if(rob->player_score > scoremanager_get_highest_score(rob->score_man)) {
          rob->is_new_highscore = true;
        } else {
          rob->is_new_highscore = false;
        }

        //Going into score submit state
        rob->currstate.state = SCORE_SUBMIT;
        //Upon winning, level is destroyed to save memory
        destroy_level(&(rob->level));
        //Game Stats as well since we already got what we wanted from it (time taken as hh:mm:ss string and calculated score)
        destroy_gamestats(&(rob->game_stats));
        //Winning the game uses a snapshot as a background
        snapshot_game(rob);
      } else {
        if(!(rob->sent_at_exit)) {
          //Otherwise just tell the other player we are at the exit
          uart_send_string(COMM_OVER_EXIT_STR);
          //To prevent spamming until overrun
          rob->sent_at_exit = true;
        }
      }
      break;
    case RECEIVED_REMOTE_MESSAGE:
      //Process messages from other player here
      //If other player leaves game
      if(strncmp(evt->remote_msg, COMM_PLAYER_LEAVING_STR, strlen(COMM_PLAYER_LEAVING_STR)) == 0) {
        //If the other player is leaving so are we
        rob->currstate.state = MENU;
        //Resetting menu before going there (Going back to main menu, etc)
        reset_menumanager(rob->menu_man);
        //When leaving also deallocate used things
        destroy_gamestats(&(rob->game_stats));
        destroy_level(&(rob->level));
      }
      //Remote lost (Collided with guard)
      if(strncmp(evt->remote_msg, COMM_PLAYER_LOST_STR, strlen(COMM_PLAYER_LOST_STR)) == 0) {
        //If other player lost, so did we
        //Destroy used objects and move into lose state
        destroy_gamestats(&(rob->game_stats));
        destroy_level(&(rob->level));
        rob->currstate.state = LOSE_MP;
      }
      //Remote got treasure (need to open exit lock)
      if(strncmp(evt->remote_msg, COMM_TREASURE_GOT_STR, strlen(COMM_TREASURE_GOT_STR)) == 0) {
        //Updates exit state
        level_remote_got_treasure(rob->level);
      }
      //Remote got coin, also tick local coins
      if(strncmp(evt->remote_msg, COMM_COIN_GOT_STR, strlen(COMM_COIN_GOT_STR)) == 0) {
        gamestats_tick_coins(rob->game_stats);
      }
      //We are both at the exit! Moving to win state
      if(strncmp(evt->remote_msg, COMM_BOTH_AT_EXIT_STR, strlen(COMM_BOTH_AT_EXIT_STR)) == 0) {
        printf("DBG: Got both at exit message! Moving to win screen!\n");
        //Get stats and move to score submit screen
        //First, we calculate player score to have it stored for display later on
        rob->player_score = gamestats_calculate_score(rob->game_stats);
        //Also saving time taken for displaying
        rob->time_taken = gamestats_get_time_taken(rob->game_stats);
        //Checking if the score is a new highscore
        if(rob->player_score > scoremanager_get_highest_score(rob->score_man)) {
          rob->is_new_highscore = true;
        } else {
          rob->is_new_highscore = false;
        }

        //Going into score submit state
        rob->currstate.state = SCORE_SUBMIT;
        //Upon winning, level is destroyed to save memory
        destroy_level(&(rob->level));
        //Game Stats as well since we already got what we wanted from it (time taken as hh:mm:ss string and calculated score)
        destroy_gamestats(&(rob->game_stats));
        //Winning the game uses a snapshot as a background
        snapshot_game(rob);
      }
      if(strncmp(evt->remote_msg, COMM_OVER_EXIT_STR, strlen(COMM_OVER_EXIT_STR)) == 0) {
        //The other player is now at the exit, remember that
        rob->other_player_at_exit = true;
        //Now, when we touch the exit we should send win string
      }
      //Abort, error ocurred
      if(strncmp(evt->remote_msg, COMM_ABORT_STR, strlen(COMM_ABORT_STR)) == 0) {
        rob->currstate.state = MENU;
        //Resetting menu before going there (Going back to main menu, etc)
        reset_menumanager(rob->menu_man);
        //When leaving also deallocate used things
        destroy_gamestats(&(rob->game_stats));
        destroy_level(&(rob->level));
      }
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

static void game_process_events_lose_mp(Robinix * rob, Event * evt) {
  switch (evt->evt_type) {
    case KEY_DOWN:
      break;
    case KEY_UP:
      switch (evt->pressed_key) {
        case '!':
          //Resetting menu before going there (Going back to main menu, etc)
          reset_menumanager(rob->menu_man);
          rob->currstate.state = MENU;
          break;
        default:
          break;
      }
      break;
    case MOUSE_MOVE:
      //Mouse was moved, updating internal positions
      rob->currstate.mouseX += evt->mouse_move_x;
      //Y is inverted because +y for the mouse is up but for us that is down
      rob->currstate.mouseY += -evt->mouse_move_y;
      //Checking if mouse is leaving screen and stopping it from doing so
      limit_xy_inside_screen(&(rob->currstate.mouseX), &(rob->currstate.mouseY), rob->mouse_bmp->bitmapInfoHeader.width, rob->mouse_bmp->bitmapInfoHeader.height);
      break;
    //No other events are being considered at the moment
    default:
      break;
  }
}

void game_process_events(Robinix * rob) {
  int i;
  for(i = 0; i < rob->n_events_to_process; i++) {
    switch (rob->currstate.state) {
      case PLAYING_SP:
        game_process_event_playing_sp(rob, rob->event_buffer[i]);
        break;
      case PAUSED_SP:
        game_process_event_paused(rob, rob->event_buffer[i]);
        break;
      case LOSE_SP:
        game_process_event_lose_sp(rob, rob->event_buffer[i]);
        break;
      case MENU:
        game_process_event_menu(rob, rob->event_buffer[i]);
        break;
      case SCORE_SUBMIT:
        game_process_events_score_submit(rob, rob->event_buffer[i]);
        break;
      case SEARCHING_MP:
        game_process_events_searching_mp(rob, rob->event_buffer[i]);
        break;
      case SYNCING_MP:
        game_process_events_syncing_mp(rob, rob->event_buffer[i]);
        break;
      case WAITING_MP:
        game_process_events_waiting_mp(rob, rob->event_buffer[i]);
        break;
      case PLAYING_MP:
        game_process_events_playing_mp(rob, rob->event_buffer[i]);
        break;
      case LOSE_MP:
        game_process_events_lose_mp(rob, rob->event_buffer[i]);
        break;
      default:
        break;
    }
  }

  //After processing events clear event array
  clear_event_buffer(rob);
}
