#include "score.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* For conversion error checking */
#include <limits.h>
#include <errno.h>

Score * create_score(unsigned long points, const char * name, const char * finish_date) {
  //Allocating and checking if allocation was successful
  Score * s_ptr = malloc(sizeof *s_ptr);

  if (s_ptr == NULL) {
    return NULL;
  }

  //Storing the passed arguments in the score object
  //NOTE: Copying the strings to prevent problems if the memory passed is free'd in the meantime
  s_ptr->points = points;
  s_ptr->name = strdup(name);
  s_ptr->finish_date = strdup(finish_date);

  if(s_ptr->name == NULL || s_ptr->finish_date == NULL) {
    //Allocation unsuccessful, free the other variables and return NULL
    free(s_ptr->name);
    free(s_ptr->finish_date);
    free(s_ptr);
    return NULL;
  }

  //Returning created score object
  return s_ptr;
}

Score * create_score_from_string(const char * s) {
  if(s == NULL) {
    return NULL;
  }

  //Preserving input by copying into new string
  char * str = strdup(s);

  if(str == NULL) {
    //Couldn't copy string or other error
    return NULL;
  }

  //Since we know a line always follows the same format we can do this (kind of hardcoded)

  //If we were to use strtok_r (because it is thread safe and strtok is not)
  //char * save_ptr;
  //However, we were not able to get the compiler to accept our code while using strtok_r,
  //since it had a lot of warnings about implicit declaration despite string.h being included
  //and _GNU_SOURCE defined in the makefile

  //Thus we have to resort to using strtok despite it being less safe

  char * points = strtok(str, " ");
  if(points == NULL) {
    //Since strtok operates on the passed original string we only need to free that one (since we allocated it)
    free(str);
    return NULL;
  }

  char * name = strtok(NULL, " ");
  if(name == NULL) {
    //Since strtok operates on the passed original string we only need to free that one (since we allocated it)
    free(str);
    return NULL;
  }

  char * date = strtok(NULL, " ");
  if(date == NULL) {
    //Since strtok operates on the passed original string we only need to free that one (since we allocated it)
    free(str);
    return NULL;
  }

  char * temp_str_endptr;
  unsigned long points_int = strtoul(points, &temp_str_endptr, 10);
  //Checking if any conversion error ocurred (either value outside of range or no valid conversion could be performed)
  if ((errno == ERANGE && points_int == ULONG_MAX) || (errno != 0 && points_int == 0)) {
    printf("DBG: Conversion problem\n");
    //Conversion error, freeing and returing NULL
    //Since strtok operates on the passed original string we only need to free that one (since we allocated it)
    free(str);
    return NULL;
  }

  if (temp_str_endptr == points) {
    //If the end pointer is the same as the starting one, then no digits were found in the passed string
    //Freeing and returing NULL
    //Since strtok operates on the passed original string we only need to free that one (since we allocated it)
    free(str);
    return NULL;
  }

  //We should now copy the name and date strings to a different variable
  //(since strtok was used, they are only parts of the same original string, freeing one will cause problems in accessing the other)
  char * final_name = strdup(name);
  char * final_date = strdup(date);

  //If any of the allocations failed, freeing and returning NULL
  if(final_name == NULL || final_date == NULL) {
    //(Freeing something that is NULL causes no problem, as free does nothing in that case)
    free(final_name);
    free(final_date);
    free(str);
  }

  //Copied string is no longer needed
  //(points string is also "free'd" since strtok was used, points was just a part of the original string)
  free(str);

  //Attempting to create the Score object
  Score * s_ptr = malloc(sizeof * s_ptr);

  if (s_ptr == NULL) {
    //Score could not be allocated, freeing and returning NULL
    free(final_name);
    free(final_date);
    return NULL;
  }

  //Storing the passed arguments in the score object
  s_ptr->points = points_int;
  s_ptr->name = final_name;
  s_ptr->finish_date = final_date;

  //Returning created score object
  return s_ptr;
}

void destroy_score(Score ** s_ptr) {
  //Score strings were allocated when constructing score object so they must be deallocated here as well
  free((*s_ptr)->name);
  free((*s_ptr)->finish_date);

  free(*s_ptr);
  *s_ptr = NULL;
}

char * score_to_string(Score * s_ptr) {
  if (s_ptr == NULL){
    printf("score_to_string::Score pointer was null\n");
    return NULL;
  }

  //50 is a size that should always be enough to prevent overrunning the buffer
  //Maximum of unsigned long is around 4.3 million which is 10 characters, the name should never be over 5 characters, the date should never be over 8 characters (or 10 if somehow the year is 4 characters)
  //Total so far = 25 + 4 for spaces + 1 for \0 = 30
  //30 plus 25 just to be on the safe side = 55
  char * score_string = malloc(55 * sizeof *score_string);
  if(score_string == NULL) {
    printf("Debug: Failure in allocation of string for score_to_string\n");
    return NULL;
  }

  //Writing the string in the desired format
  //(The score and name are (left) space padded just to look better in highscore menu, create_score_from_string uses strtok so saving with extra spaces poses no problem)
  sprintf(score_string, "%-3lu  %-5s  %s", s_ptr->points, s_ptr->name, s_ptr->finish_date);

  //Truncating the string to prevent memory waste
  if(strlen(score_string) < 55) {
    //+1 for \0 - never forget your \0 s
    char * temp_score_string = realloc(score_string, (strlen(score_string) + 1) * sizeof *score_string);
    if(temp_score_string != NULL) {
      //Reallocation successful
      score_string = temp_score_string;
      //The previous pointer is free'd by realloc so no need to free it here
    }
  }


  //Everything went as expected
  return score_string;
}
