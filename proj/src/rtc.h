#ifndef __RTC_H
#define __RTC_H

#include <stdbool.h>
#include "utilities.h"
#include "rtc_defines.h"

/** @defgroup rtc rtc
 * @{
 *
 * RTC Device Driver main implementation and Date object operations
 */

typedef struct {
  unsigned long year;
  unsigned long month;
  unsigned long day;
  unsigned long hour;
  unsigned long minute;
  unsigned long second;
} Date_obj;

/**
 * @brief Assembly interrupt handler to read register C
 * @return The return value is irrelevant, the function returns via the "asm_regC" variable
 */
extern unsigned long asm_rtc_ih();
extern unsigned long asm_regC;

/**
 * @brief Subscribes RTC interrupts
 * @return Returns 0 if succesful, not 0 otherwise
 */
int rtc_subscribe_int();

/**
 * @brief Unsubscribes RTC interrupts
 * @return Returns 0 if successful, not 0 otherwise
 */
int rtc_unsubscribe_int();

/**
 * @brief RTC Interrupt Handler - Reads register C (using assembly IH) and acts accordingly
 * @return Returns 0 if successful, not 0 otherwise
 */
int rtc_IH();

/**
 * @brief Activates RTC Update Interrupts
 * @return Returns 0 if successful, not 0 otherwise
 */
int rtc_activate_update_interrupts();

/**
 * @brief Deactivates RTC Update Interrupts
 * @return Returns 0 if successful, not 0 otherwise
 */
int rtc_deactivate_update_interrupts();

/**
 * @brief Verifies the RTC current settings and converts the passed in data from reading the RTC registers from BCD to binary, if necessary
 * @param  n Data to convert, if necessary
 * @return   Converted data
 */
unsigned long data_bcd_to_binary(unsigned long n);

/**
 * @brief Calculates the seconds elapsed between two dates
 * @param  d1 Date 1
 * @param  d2 Date 2
 * @return    Number of seconds elapsed between two dates
 */
int get_seconds_elapsed_between_dates(Date_obj d1, Date_obj d2);

/**
 * @brief Gets the number of seconds elapse since 00:00:00 (Only considers HH:MM:SS since this is used during gameplay, where a Date_obj is used as a time counter)
 * @param  d1 Date object to get the number of seconds for
 * @return    The number of seconds in the passed Date
 */
int get_seconds(const Date_obj * d1);

/**
 * @brief Increments time by 1 second in the passed Date
 * @param date Date to increment
 */
void tick_second(Date_obj * date);

/**
 * @brief Checks if the curr_date (Date_obj) is between the 18th of December and 6th January
* @return Returns true if the date is between (XXXX/12/18) and (XXXX/01/06) or false otherwise
 */
bool is_christmas_time();

/**
 * @brief Returns a copy of the current Date, kept updated using RTC update interrupts
 * @return Returns a copy of the current Date
 */
Date_obj get_current_date();

/**
 * @brief Waits until the current date can be read, was used for polling
 */
void wait_RTC();

/**
 * @brief Updates currently stored date, to be used when Register C denotes an update interrupt
 * @return Returns 0 if successful, not 0 otherwise
 */
int update_RTC_date();

/**
 * @brief Converts a Date to a date string in the YYYY/MM/DD format
 * @param  date Date to convert to date string
 * @return      Returns a string in the correct date format
 */
char * date_to_date_string(const Date_obj * date);

/**
 * @brief Converts a Date to a time string in the HH:MM:SS format
 * @param  date Date to convert to time string
 * @return      Returns a string in the correct time format
 */
char * date_to_time_string(const Date_obj * date);

/**
 * @brief Converts a Date to a date and time string in the YYYY/MM/DD HH:MM:SS format
 * @param  curr_date Date to convert to string
 * @return           Returns a string representative of the passed date
 */
char* date_to_string(const Date_obj* curr_date);

#endif /* __RTC_H */
