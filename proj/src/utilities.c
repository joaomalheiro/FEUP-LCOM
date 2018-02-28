#include "utilities.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

unsigned long long parse_ullong(char *str, int base) {
  char *endptr;
  unsigned long long val;

  /* Convert string to unsigned long */
  val = strtoull(str, &endptr, base);

  /* Check for conversion errors */
  if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
    printf("parse_ullong::Error, values out of range!\n");
    return ULLONG_MAX;
  }

  if (endptr == str) {
    printf("parse_ullong::no digits were found in %s\n", str);
    return ULLONG_MAX;
  }

  /* Successful conversion */
  return val;
}
