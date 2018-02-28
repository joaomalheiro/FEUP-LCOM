#include "rtc.h"
#include "rtc_defines.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/driver.h>
//#include <string.h>
#include <math.h>
#include "i8042.h"

////Private variables
//Hook ID for subscribing to rtc interrupts
static int rtc_hookID = 8;

static Date_obj curr_date = {.year = 0, .month = 0, .day = 0, .hour = 0, .minute = 0, .second = 0};


int rtc_subscribe_int() {

  //Variable used for preserving rtc_hookID value that will later be used
	int temp = rtc_hookID;

	if(sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &rtc_hookID) != OK) {
		printf("rtc_subscribe_int::Error setting IRQ policy!\n");
		return -1;
	}

	if(sys_irqenable(&rtc_hookID)!= OK){
		printf("rtc_subscribe_int::Error enabling interrupts on the IRQ line!\n");
		return -2;
	}

	if(rtc_activate_update_interrupts() != 0) {
		printf("rtc_subscribe_int::Error activating update interrupts!\n");
		return -3;
	}

	//Everything went as expected, returning bitmask of the rtc_hookID for interrupt handling
	return BIT(temp);
}

int rtc_unsubscribe_int() {

	if(sys_irqdisable(&rtc_hookID) != OK){
		printf("rtc_unsubscribe_int::Error disabling interrupts on the IRQ line\n");
		return 1;
	}

	if(sys_irqrmpolicy(&rtc_hookID) != OK){
		printf("rtc_unsubscribe_int::Error unsubscribing the previous subscription of the interrupt notification on the IRQ line!\n");
		return 2;
	}

	if(rtc_deactivate_update_interrupts() != 0) {
		printf("rtc_unsubscribe_int::Error deactivating update interrupts!\n");
		return 3;
	}

	//Everything went as expected
	return 0;
}

int rtc_activate_update_interrupts() {

 	unsigned long regB = 0;

  if(sys_outb(RTC_ADDRESS_REG, RTC_REG_B) != OK) {
    printf("rtc_enable_interrupts::error requesting register B reading");
		return -1;
  }

  if (sys_inb(RTC_DATA_REG, &regB) != OK) {
    printf("rtc_enable_interrupts::Error reading register B\n");
		return -2;
	}

	regB |= RTC_REG_B_UIE;
	//To ensure that the RTC is being updated, we also unset the SET bit
	regB &= ~RTC_REG_B_SET;

	if(sys_outb(RTC_ADDRESS_REG, RTC_REG_B) != OK) {
    printf("rtc_enable_interrupts::error requesting register B writing");
		return -3;
  }

	if(sys_outb(RTC_DATA_REG, regB) != OK) {
    printf("rtc_enable_interrupts::error writing Register B");
		return -4;
  }

  //Everything went as expected
  return 0;
}

int rtc_deactivate_update_interrupts() {

 	unsigned long regB = 0;

  if(sys_outb(RTC_ADDRESS_REG, RTC_REG_B) != OK) {
    printf("rtc_enable_interrupts::error requesting register B reading");
		return -1;
  }

  if (sys_inb(RTC_DATA_REG, &regB) != OK) {
    printf("rtc_enable_interrupts::Error reading register B\n");
		return -2;
	}

	regB &= ~BIT(4);

	if(sys_outb(RTC_ADDRESS_REG, RTC_REG_B) != OK) {
    printf("rtc_enable_interrupts::error requesting register B writing");
		return -3;
  }

	if(sys_outb(RTC_DATA_REG, regB) != OK) {
    printf("rtc_enable_interrupts::error writing Register B");
		return -4;
  }

  //Everything went as expected
  return 0;
}

int rtc_IH() {
	//Reading Register C to clear interrupts flag and see which kind of interrupt we received
	unsigned long regC = 0;

	/*if(sys_outb(RTC_ADDRESS_REG, RTC_REG_C) != OK) {
		printf("rtc_IH::error requesting register C reading");
		return -1;
	}
	
	if (sys_inb(RTC_DATA_REG, &regC) != OK) {
		printf("rtc_IH::Error reading register C\n");
		return -2;
	}
	*/

	asm_rtc_ih();
	regC = asm_regC;

	//Periodic interrupt
	if(regC & RTC_REG_C_PF) {
		printf("rtc_IH::Received periodic interrupt\n");
	}

	//Alarm interrupt
	if(regC & RTC_REG_C_AF) {
		printf("rtc_IH::Received alarm interrupt\n");
	}

	//Update interrupt
	if(regC & RTC_REG_C_UE) {
		if(update_RTC_date() != 0) {
			printf("rtc_IH::Error in handling update interrupt\n");
			return -3;
		}
	}

	return 0;
}

void wait_RTC() {

  unsigned long regA = 0;

  do {
    if(sys_outb(RTC_ADDRESS_REG, RTC_REG_A) != OK) {
      printf("wait_RTC::error requesting register A reading");
    }
    if (sys_inb(RTC_DATA_REG, &regA) != OK) {
      printf("wait_RTC::Error reading register A\n");
    }
  } while(regA & RTC_REG_A_UIP);
}

unsigned long data_bcd_to_binary(unsigned long n){
	unsigned long bin = 0;
	unsigned long regB = 0;

	if(sys_outb(RTC_ADDRESS_REG, RTC_REG_B) != OK) {
		printf("wait_RTC::error requesting register A reading");
	}
	if (sys_inb(RTC_DATA_REG, &regB) != OK) {
		printf("wait_RTC::Error reading register A\n");
	}

	if (!(regB & RTC_REG_B_DM)){
		bin = ((n & 0xF0) >> 4) * 10 + (n & 0x0F);
		return bin;
	}
	return n;
}

bool is_christmas_time() {

	Date_obj date = get_current_date();

	return ((date.month == 12 && (date.day >= 18 && date.day <= 31)) || (date.month == 1 && (date.day >= 1 && date.day <= 6)));

}

void tick_second(Date_obj * date) {
	if (date->second == 59) {
		date->second = 0;
		if (date->minute == 59){
			date->minute = 0;
			if (date->hour == 24){
				date->hour = 0;
				date->day++;
			} else {
				date->hour++;
			}
		} else {
			date->minute++;
		}
	} else {
		date->second++;
	}
}

int get_seconds_elapsed_between_dates(Date_obj d1, Date_obj d2) {

	Date_obj passed_time;


	int days_per_month = 0;
	int days_per_year = 0;
	//Calculating how many days there are in the month that possibly passed and how many days there are in the year that possibly passed
	if(d1.month == 4 || d1.month == 6 || d1.month ==9 || d1.month == 11){
		days_per_month = 30;

	}	else if(d1.month == 2){
		if(d1.year % 4 == 0){
			days_per_month = 29;
			days_per_year = 366;

		} else {
			days_per_month = 28;
			days_per_year = 365;
		}
	} else 	if (d1.month == 1 || d1.month ==3 || d1.month==5 || d1.month ==7 || d1.month ==8 || d1.month ==10 || d1.month==12){
		days_per_month = 31;
	}


	if(d2.second > d1.second){
		--d1.minute;
		d1.second += 60;
	}

  passed_time.second = d1.second - d2.second;
  if(d2.minute > d1.minute){
      --d1.hour;
      d1.minute += 60;
  }

  passed_time.minute = d1.minute - d2.minute;

	if(d2.hour > d1.hour){
      --d1.day;
      d1.hour += 24;
  }

  passed_time.hour = d1.hour - d2.hour;

	if (d2.day > d1.day) {
		--d1.month;
		d1.day += days_per_month;
		}

	passed_time.day = d1.day - d2.day;

	if (d2.month > d1.month) {
		--d1.year;
		d1.month += 12;
	}

	passed_time.month = d1.month - d2.month;
	passed_time.year = d1.year - d2.year;

	int a = (passed_time.year)*days_per_year*24*60*60 + (passed_time.month)*days_per_month*24*60*60 + (passed_time.day)*24*60*60 +(passed_time.hour)*60*60 + (passed_time.minute)*60 + (passed_time.second);
	return a;
}

int get_seconds(const Date_obj * d1) {
	if(d1 == NULL) {
		return -1;
	}
	return d1->hour * 60*60 + d1->minute * 60 + d1->second;
}

char * date_to_date_string (const Date_obj * date) {

	if (date == NULL) {
		return NULL;
	}

	//Date string is at most YYYY:MM:DD so 10 characters + 1 for \0 = 11 + 3 for safety = 14

	char * rtc_string = malloc(14 * sizeof *rtc_string);

	if(rtc_string == NULL) {
		//Could not be allocated
		return NULL;
	}

	sprintf(rtc_string, "20%02d/%02d/%02d", date->year, date->month, date->day);

	return rtc_string;
}

char * date_to_time_string(const Date_obj * date) {

	if (date == NULL) {
		return NULL;
	}

	//Time string is at most HH:MM:SS so 8 characters + 1 for \0 = 9 + 3 for safety = 12

	char * rtc_string = malloc(12 * sizeof *rtc_string);

	if(rtc_string == NULL) {
		//Could not be allocated
		return NULL;
	}

	sprintf(rtc_string, "%02d:%02d:%02d", date->hour, date->minute, date->second);

	return rtc_string;
}

char* date_to_string(const Date_obj* date) {
    if (date == NULL) {
      return NULL;
    }

		//Total date string is 8 for time + 10 for date + 1 for space + 1 for \0 = 20 + 6 for safety = 26

		char * rtc_string = malloc(26 * sizeof *rtc_string);

		if(rtc_string == NULL) {
			//Could not be allocated
			return NULL;
		}

    sprintf(rtc_string, "20%02d/%02d/%02d %02d:%02d:%02d", date->year, date->month, date->day, date->hour, date->minute, date->second);

    return rtc_string;
}

Date_obj get_current_date() {
  return curr_date;
}

int update_RTC_date() {
  //wait_RTC();

	unsigned long temp_var = 0;

  if(sys_outb(RTC_ADDRESS_REG, RTC_YEAR) != OK) {
    printf("getRTCinfo::Error requesting year reading");
		return -1;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading year\n");
		return -2;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.year = temp_var;


  if(sys_outb(RTC_ADDRESS_REG, RTC_MONTH) != OK) {
    printf("getRTCinfo::Error requesting month reading");
		return -3;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading month\n");
		return -4;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.month = temp_var;

  if(sys_outb(RTC_ADDRESS_REG, RTC_DAY) != OK) {
    printf("getRTCinfo::Error requesting day reading");
		return -5;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading day\n");
		return -6;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.day = temp_var;

  if(sys_outb(RTC_ADDRESS_REG, RTC_HOUR) != OK) {
    printf("getRTCinfo::Error requesting hour reading");
		return -7;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading hour\n");
		return -8;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.hour = temp_var;

  if(sys_outb(RTC_ADDRESS_REG, RTC_MINUTE) != OK) {
    printf("getRTCinfo::Error requesting minute reading");
		return -9;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading minute\n");
		return -10;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.minute = temp_var;

  if(sys_outb(RTC_ADDRESS_REG, RTC_SECOND) != OK) {
    printf("getRTCinfo::Error requesting second reading");
		return -11;
  }
  if (sys_inb(RTC_DATA_REG, &temp_var) != OK) {
    printf("getRTCinfo::Error reading second\n");
		return -12;
  }

	temp_var = data_bcd_to_binary(temp_var);
	curr_date.second = temp_var;

	return 0;
}
