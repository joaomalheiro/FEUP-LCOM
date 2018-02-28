#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <minix/drivers.h>
//User includes
#include "mouse.h"
#include "test4.h"
#include "i8042.h"

static int proc_args(int argc, char **argv);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char **argv);

int main(int argc, char **argv) {
  /* initializing service */
	sef_startup();

  if (argc == 1) {					/* Prints usage of the program if no arguments are passed */
		print_usage(argv);
		return 0;
	}
	else return proc_args(argc, argv);
}

static void print_usage(char **argv) {
	printf("Usage: one of the following:\n"
			"\t service run %s -args \"packet <decimal no. - number of packets to receive>\"\n"
			"\t service run %s -args \"async <decimal no. - seconds without packets until break>\"\n"
			"\t service run %s -args \"remote <decimal no. - period in ms> <decimal no. - number of packets to receive>\"\n"
			"\t service run %s -args \"gesture <decimal no. - minimum length of movement in the x direction>\"\n",
			argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char **argv) {
	if (strncmp(argv[1], "packet", strlen("packet")) == 0) {
		if (argc != 3) {
			printf("mouse: wrong no. of arguments for mouse_test_packet()\n");
			return 1;
		}
		unsigned long pktcnt = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
		if (pktcnt == ULONG_MAX)
			return 1;
		printf("mouse::mouse_test_packet(%lu)\n", pktcnt);
		return mouse_test_packet(pktcnt);
	}
	else if (strncmp(argv[1], "async", strlen("async")) == 0) {
		if (argc != 3) {
			printf("mouse: wrong no. of arguments for mouse_test_async()\n");
			return 1;
		}
		unsigned long nsecs = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
		if (nsecs == ULONG_MAX)
			return 1;
		printf("mouse::mouse_test_async(%lu)\n", nsecs);
		return mouse_test_async(nsecs);
	}
	else if (strncmp(argv[1], "remote", strlen("remote")) == 0) {
		if (argc != 4) {
			printf("mouse: wrong no. of arguments for mouse_test_remote()\n");
			return 1;
		}
		unsigned long period = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
		if (period == ULONG_MAX)
			return 1;
		unsigned long pktcnt = parse_ulong(argv[3], 10);						/* Parses string to unsigned long */
		if (pktcnt == ULONG_MAX)
			return 1;
		printf("mouse::mouse_test_remote(%lu, %lu)\n", period, pktcnt);
		return mouse_test_remote(period, pktcnt);
	}
	else if(strncmp(argv[1], "gesture", strlen("gesture")) == 0){
		if (argc != 3) {
			printf("mouse: wrong no. of arguments for mouse_test_gesture()\n");
			return 1;
		}
		long minlen = parse_long(argv[2], 10);						/* Parses string to unsigned long */
		if (minlen == LONG_MAX)
			return 1;
		printf("mouse::mouse_test_gesture(%ld)\n", minlen);
		return mouse_test_gesture(minlen);
	}	else {
		printf("mouse: %s - no valid function!\n", argv[1]);
		return 1;
	}
}

static unsigned long parse_ulong(char *str, int base) {
	char *endptr;
	unsigned long val;

	/* Convert string to unsigned long */
	val = strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("parse_ulong: no digits were found in %s\n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}

static long parse_long(char *str, int base) {
	char *endptr;
	long val;

	/* Convert string to long */
	val = strtol(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
		perror("strtol");
		return LONG_MAX;
	}

	if (endptr == str) {
		printf("parse_long: no digits were found in %s\n", str);
		return LONG_MAX;
	}

	/* Successful conversion */
	return val;
}
