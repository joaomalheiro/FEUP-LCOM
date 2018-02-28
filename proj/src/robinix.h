#ifndef __ROBINIX_H
#define __ROBINIX_H

#include <stdbool.h>
#include "bitmap.h"
#include "sprite.h"
#include "level.h"
#include "menumanager.h"
#include "scoremanager.h"
#include "gamestats.h"

/** @defgroup robinix robinix
 * @{
 *
 * Functions and structs for main operations of the project (Game Object operation)
 */

//Events enum
typedef enum {
  //Peripheral events
  MOUSE_LB_DOWN = 0,
  MOUSE_MB_DOWN,
  MOUSE_RB_DOWN,
  MOUSE_MOVE,
  KEY_DOWN,
  KEY_UP,
  //For Serial Port messages
  RECEIVED_REMOTE_MESSAGE,

  ////"Higher level" events
  //For menu
  CLICKED_EXIT_GAME,
  CLICKED_PLAY_MULTI,
  CLICKED_PLAY_LVL_1,
  CLICKED_PLAY_LVL_2,
  //For ingame
  PLAYER_COLLIDE_WITH_GUARD,
  PLAYER_GOT_TREASURE,
  PLAYER_GOT_COIN,
  PLAYER_COLLIDE_WITH_EXIT,
  //For MP ingame
  //TBD because these will just be serial port messages so they can be treated straight away like that
  /*
  REMOTE_GOT_TREASURE,
  REMOTE_PRESSED_BUTTON1
  */
} event_enum;

//Events struct
typedef struct {
  //Of what type the event is
  event_enum evt_type;
  //How much the mouse was moved in the x and y axis
  int mouse_move_x;
  int mouse_move_y;
  //Which keyboard key was pressed (already in the correct char value)
  char pressed_key;
  //Remote string received through Serial Port
  char * remote_msg;
} Event;

//Game states enum
typedef enum {
  MENU = 0,
  PLAYING_SP,
  PAUSED_SP,
  LOSE_SP,
  SCORE_SUBMIT,
  SCORE_LIST,

  SEARCHING_MP,
  SYNCING_MP,
  WAITING_MP,
  PLAYING_MP,
  LOSE_MP,

  EXIT_GAME
} state_enum;

//States struct
typedef struct {
  //These 2 are signed because of checking if the mouse is "off the screen" and putting it back in the screen
  long mouseX;
  long mouseY;
  //For knowing if a key is down or up
  bool w_pressed;
  bool a_pressed;
  bool s_pressed;
  bool d_pressed;
  //The indicator of the current state
  state_enum state;
} State;

///Game setting constants
#define PLAYER_NAME_MAX_LENGTH          5
///Communication
#define COMM_MSG_RETRY_TICKDELAY        15 /* 4 times per second */
#define COMM_DELAY_UNTIL_PINGING        60 /* The delay to wait until starting to send beacon messages */
#define COMM_TICK_SYNC_DELAY            10 /* The delay in which to send sync ticks */
#define COMM_DELTA_FOR_HANDSHAKE        239 /* Number of ticks to agree to start in (4 seconds minus 1 tick at the moment) */
#define COMM_TICK_SYNC_MIN              6 /* Minimum number of concurrent successfully synced ticks to consider the program synced */
#define COMM_SEARCHING_STR              "any1there" /* "Beacon" message, to seach for player 2 */
#define COMM_REPLY_STR                  "im here" /* Reply to the "beacon" */
#define COMM_ACKNOWLEDGE_REPLY          "gotcha" /* To reply to the reply and establish connection */
#define COMM_SYNC_PREFIX                "sk" /* Prefix to identify a timer sync value */
#define COMM_SYNCED_STR                 "nsync" /* When synced correctly, message to pass */
#define COMM_START_TICK_PREFIX          "when i say go" /* Prefix to the agreed tick to start in */
#define COMM_TREASURE_GOT_STR           "gottem"
#define COMM_COIN_GOT_STR               "ding"
#define COMM_OVER_EXIT_STR              "im at soup"
#define COMM_BOTH_AT_EXIT_STR           "winning"
#define COMM_PLAYER_LOST_STR            "i died"
#define COMM_PLAYER_LEAVING_STR         "im out"
#define COMM_ABORT_STR                  "cancela" /* In case of a critical error that should abort the process this is sent */

typedef enum {
  COMM_WAITING_TO_PING = 0,
  COMM_PINGING,
  COMM_REPLYING,
  COMM_ACKING,
  COMM_SYNCING,
  COMM_IG_NONE
} comm_state_enum;

//Robinix game struct
//Player and guard struct defined in the respective headers
//The variables that are ** are arrays of pointers, because it is not possible to have flexible array members not at the end of a struct
//(thus it is not possible to have two arrays in the same struct, therefore we use **)
typedef struct Robinix {
  //Holds relevant information about game current state
  State currstate;
  //Buffer of events to process ('array of events')
  //Needs to be pointers because of deallocating memory
  Event ** event_buffer;
  //Indicates the size of the event buffer array above
  unsigned int n_events_to_process;
  //Peripheral subscriptions
  int timer_irq_bitmask;
  int keyboard_irq_bitmask;
  int mouse_irq_bitmask;
  int rtc_irq_bitmask;
  int uart_irq_bitmask;

  //Bitmap of the mouse pointer, used for menus
  Bitmap * mouse_bmp;
  //Bitmap * mouse_menu_bmp;
  struct MenuManager * menu_man;

  //Used for when the game is paused, or you either win or lose
  unsigned short * snapshot_buffer;
  //Pause menu bitmap
  Bitmap * pause_menu_bmp;
  //Win Screen bitmap
  Bitmap * win_screen_bmp;
  //Lose Screen Sprite
  Sprite * lose_screen_sprite;

  //Object for managing scores
  ScoreManager * score_man;

  //Helper variable for counting timer ticks while playing (to know when a second has passed)
  unsigned int timer_ticks_playing;
  //Object for calculating scores and managing game stats
  GameStats * game_stats;
  //Helper variables for game stat calculation before submission
  char player_name[PLAYER_NAME_MAX_LENGTH + 1]; //+1 to have space for the null terminator, forgot that previously, oops
  unsigned long player_score;
  char * current_date_string;
  char * time_taken;
  bool is_new_highscore;

  //Object for all types of level handling
  struct Level * level;

  ////Helper variables for multiplayer
  //Used for checking the delay when sending searching messages and also for delay when sending timer ticks to sync
  comm_state_enum comm_state;
  unsigned int mp_msg_delay_ticks;
  bool isPlayer1;
  unsigned long long mp_syncing_ticks;
  unsigned int n_ticks_synced;
  //The tick that both agreed on to start the game
  unsigned long long tick_decided;
  bool other_player_at_exit;
  bool sent_at_exit;
} Robinix;

//Subscription macros (for easier debugging - knowing exactly what happened)
#define SUBS_OK            0
#define TIMER_SUB_ERROR   -1
#define KB_SUB_ERROR      -2
#define MOUSE_SUB_ERROR   -3
#define RTC_SUB_ERROR     -4
#define UART_SUB_ERROR    -5

#define UNSUBS_OK            0
#define TIMER_UNSUB_ERROR   -1
#define KB_UNSUB_ERROR      -2
#define MOUSE_UNSUB_ERROR   -3
#define RTC_UNSUB_ERROR     -4
#define UART_UNSUB_ERROR    -5

//Other macros
#define MOUSE_START_X 350
#define MOUSE_START_Y 250

///Functions:

/**
 * @brief Robinix Object Constructor
 * @return Returns a pointer to a valid Robinix Object or NULL in case of failure
 */
Robinix * create_robinix();

/**
 * @brief Does peripheral subscription and enabling for the passed Robinix object
 * @param  rob Object to activate subscribes on
 * @return     0 if successful, not 0 otherwise
 */
int subscribe_peripherals(Robinix * rob);

/**
 * @brief Does peripheral unsubscription and disabling for the passed Robinix object
 * @param  rob Object to deactivate subscribes on
 * @return     0 if successful, not 0 otherwise
 */
int unsubscribe_peripherals(Robinix * rob);

/**
 * @brief Robinix Object destructor. A ** is passed because the pointer itself is set to NULL, to prevent dangling pointers and to know if the object is truly deallocated
 * @param rob Robinix Object to deallocate
 */
void destroy_robinix(Robinix ** rob);

////Events Object functions
/**
 * @brief Adds the passed Event to the passed Robinix event buffer (Note: Does not add the Event if it is NULL)
 * @param rob Robinix Object to add the event to
 * @param evt Event Object to add to the Robinix Object event buffer
 */
void add_event_to_buffer(Robinix * rob, Event * evt);
/**
 * @brief Event Object Constructor
 * @param  evt_type     The type of the event
 * @param  mouse_move_x How much the mouse has moved in the X coordinate
 * @param  mouse_move_y How much the mouse has moved in the Y coordinate
 * @param  pressed_key  Which key was pressed (interpreted into the correct character)
 * @param  remote_msg   The message that was received through the UART
 * @return              Returns a pointer to a valid Event object or NULL in case of failure
 */
Event * create_event(event_enum evt_type, int mouse_move_x, int mouse_move_y, char pressed_key, char * remote_msg);
/**
 * @brief Event Object Destructor
 * @param evt Event to destroy
 */
void destroy_event(Event ** evt);
/**
 * @brief Clears the Event buffer for the passed Robinix Object, destroying all the Events
 * @param rob Robinix Object in which to clear the event buffer
 */
void clear_event_buffer(Robinix * rob);

/**
 * @brief Gets the current game state
 * @param  rob Robinix Object for which to know the current game state
 * @return     Returns the current game state
 */
state_enum get_game_state(Robinix * rob);

/**
 * @brief Processes all the Events in the Robinix Object Event buffer by calling appropriate event processing functions depending on the current state
 * @param rob Robinix Object to process events for
 */
void game_process_events(Robinix * rob);

/**
 * @brief Updates the game for every timer interrupt: Sends remote messages, moves player and guards, etc
 * @param rob Robinix Object for which to update
 */
void game_update(Robinix * rob);

/**
 * @brief Draws the game based on the current game state
 * @param rob Robinix Object to draw, depending on current state
 */
void game_draw(Robinix * rob);

/**
 * @brief Loads a certain level
 * @param  rob   Robinix Object for which to load the level
 * @param  level Level ID to load
 * @param  is_mp If the level will be multiplayer or not
 * @return       0 if successful, not 0 otherwise
 */
int game_load_level(Robinix * rob, int level, bool is_mp);

/** @} */


#endif /* __ROBINIX_H */
