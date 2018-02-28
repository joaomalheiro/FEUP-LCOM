//#define _GNU_SOURCE /* for using asprintf */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <minix/drivers.h>
//User includes
#include "game.h"

static int proc_args(int argc, char **argv);
//static unsigned long parse_ulong(char *str, int base);
//static long parse_long(char *str, int base);
static void print_usage(char **argv);

int main(int argc, char **argv) {
  /* initializing service */
  sef_startup();
  //For Assembly IHs
  sys_enable_iop(SELF);

  if (argc == 1) {					/* Prints usage of the program if no arguments are passed */
    print_usage(argv);
    return 0;
  } else
    return proc_args(argc, argv);
}

static void print_usage(char **argv) {
  printf("Usage: one of the following:\n"
          "\t service run %s -args \"play\"\n"
          "\t service run %s -args \"uart <tx | rx> <string - text, if tx>\"\n"
          , argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char **argv) {
  if(strncmp(argv[1], "play", strlen("play")) == 0) {
    //
    if (argc != 2) {
      printf("robinix: wrong no. of arguments for play_game()\n");
      return 1;
    }

    printf("robinix::play_game()\n");
    return play_game();
  } else if(strncmp(argv[1], "uart", strlen("uart")) == 0) {
    //
    if (argc < 3) {
      printf("robinix: wrong no. of arguments for test_uart()\n");
      return 1;
    }

    if(strncmp(argv[2], "rx", strlen("rx")) == 0) {
      printf("robinix::test_uart_rx()\n");
      return test_uart_rx();
    } else if(strncmp(argv[2], "tx", strlen("tx")) != 0) {
      printf("robinix: wrong argument for test_uart(), was not rx or tx\n");
      return 2;
    }

    if (argc < 4) {
      printf("robinix: wrong no. of arguments for test_uart_tx()\n");
      return 3;
    }

    //Allocating buffer for the string
    char text[256] = {'\0'};

    ////Filling said buffer with arguments

    //First is added outside because of the spaces in the sequence
    strcpy(text, argv[3]);

    //Therefore we start ahead by 1 index
    int i;
    for(i = 4; i < argc; i++) {
      strcat(text, " ");
      strcat(text, argv[i]);
    }

    printf("robinix::test_uart_tx(%s)\n", text);
    return test_uart_tx(text);

  } else {
    printf("robinix: %s - no valid function!\n", argv[1]);
    return 1;
  }
}

/*
static long parse_long(char *str, int base) {
  char *endptr;
  long val;

  // Convert string to long
  val = strtol(str, &endptr, base);

  // Check for conversion errors
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
    perror("strtol");
    return LONG_MAX;
  }

  if (endptr == str) {
    printf("parse_long: no digits were found in %s\n", str);
    return LONG_MAX;
  }

	// Successful conversion
	return val;
}

static unsigned long parse_ulong(char *str, int base) {
  char *endptr;
  unsigned long val;

  // Convert string to unsigned long
  val = strtoul(str, &endptr, base);

  // Check for conversion errors
  if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
    perror("strtoul");
    return ULONG_MAX;
  }

  if (endptr == str) {
    printf("parse_ulong: no digits were found in %s\n", str);
    return ULONG_MAX;
  }

  // Successful conversion
  return val;
}
*/
