#include "scoremanager.h"
#include "score.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "font.h"


static void sort_scores(ScoreManager * sm){

  if(sm == NULL || sm->n_scores <= 0) {
    return;
  }

  int i ;
  int j;

  for(i = sm->n_scores - 1; i > 0; i--) {
    bool swapped = false;

    for (j = 0; j < i; j++) {
      if(sm->scores[j+1]->points > sm->scores[j]->points) {
        SWAP(Score *, sm->scores[j], sm->scores[j+1]);
        swapped = true;
      }
    }
    if (!swapped) return;
  }
}

///Constructs a ScoreManager based on file contents
//Errors are returned through the int passed by reference, 0 is no errors ocurred, != 0 is otherwise
//If the errorlevel is non-zero then the returned value is NULL and nothing extra needs to be deallocated, guaranteed
static ScoreManager * load_scores_from_file(int * errorlevel) {
  ScoreManager * sm = malloc(sizeof *sm);
  if(sm == NULL) {
    *errorlevel = -1;
    return NULL;
  }

  sm->n_scores = 0;
  sm->scores = NULL;

  FILE *file_ptr = fopen(SCORES_TXT_LOCATION, "r");

  if(file_ptr == NULL) {
    //Could not open file, return 0 (no scores exist yet) and NULL
    //The errors when opening file is basically only if the file does not exist yet, so we consider there to be no scores yet
    //We are not using multithreading so no necessity to worry about data races, so this assumption can be considered
    *errorlevel = 0;
    return sm;
  }

  char line[LINE_MAX_LENGTH];

  while(fgets(line, LINE_MAX_LENGTH, file_ptr)) {
    //Removing the trailing newline (fgets also copies the newline) by inserting a NULL terminator as the last character
    line[strlen(line) - 1] = '\0';
    Score * s_ptr = create_score_from_string(line);

    if(s_ptr == NULL) {
      printf("DBG: s_ptr was NULL\n");
      //Score could not be created correctly
      destroy_scoremanager(&sm);
      //Also close file
      fclose(file_ptr);
      *errorlevel = -2;
      return sm;
    }

    //No need to free allocated line since it was stack allocated

    //Score allocated correctly, count up scores and allocate space for one more element (and add the score to the array)
    Score ** score_arr_temp = realloc(sm->scores, (sm->n_scores + 1) * sizeof *(sm->scores));
    if(score_arr_temp == NULL) {
      //More scores could not be allocated correctly
      printf("Error creating score manager, score array could not be reallocated\n");
      //Destroy scores allocated until now and return -1 and NULL
      destroy_score(&s_ptr);
      destroy_scoremanager(&sm);
      //Also close file
      fclose(file_ptr);

      *errorlevel = -3;
      return sm;
    } else {
      //Score array reallocated correctly
      sm->scores = score_arr_temp;
      sm->n_scores++;
      //Adding created score at the end of the score array
      sm->scores[sm->n_scores - 1] = s_ptr;
    }
  }

  //Closing file and returning with an error level of 0
  fclose(file_ptr);
  *errorlevel = 0;
  return sm;
}

ScoreManager* create_scoremanager() {
  ScoreManager * sm_ptr = NULL;
  int errorlevel = 0;
  sm_ptr = load_scores_from_file(&errorlevel);
  //If errorlevel is != 0, sm_ptr is guaranteed to return NULL as well so no further deallocations are required
  if(errorlevel != 0) {
    return NULL;
  }

  sort_scores(sm_ptr);

  //Everything went as expected
  return sm_ptr;
}

void destroy_scoremanager(ScoreManager ** sm_ptr) {
  if(*sm_ptr == NULL) {
    return;
  }

  //Destroying scores
  int i;
  for(i = 0; i < (*sm_ptr)->n_scores; i++) {
    destroy_score(&((*sm_ptr)->scores[i]));
  }

  //Deallocating object itself
  free(*sm_ptr);
  *sm_ptr = NULL;
}

//Useful since we are always appending to the file (Must clear first)
static void clear_scores_file() {
  FILE * fp = fopen(SCORES_TXT_LOCATION, "w");
  if(fp != NULL) {
    fclose(fp);
  }
}

static void write_line_to_scores_file(char * text) {
  if(text == NULL) {
    return;
  }

  FILE *fp = fopen(SCORES_TXT_LOCATION, "a");
  if (fp != NULL) {
    fprintf(fp, "%s\n",text);
    fclose (fp);
  }
}

void write_scores_to_scores_file(ScoreManager * sm) {
  clear_scores_file();

  char * tempstring;

  int i;
  for(i = 0; i < sm->n_scores; i++) {
    tempstring = score_to_string(sm->scores[i]);
    write_line_to_scores_file(tempstring);
    free(tempstring);
  }
}

static void add_scoreobj_to_scoremanager(ScoreManager * sm, Score * s) {
  if(sm == NULL || s == NULL) {
    return;
  }

  //Attempting to grow the score array by 1
  Score ** temp = realloc(sm->scores, (sm->n_scores + 1) * sizeof *(sm->scores));
  if(temp != NULL) {
    //If the returned pointer is not NULL, reallocation was successful
    sm->scores = temp;
    sm->n_scores++;
    sm->scores[sm->n_scores - 1] = s;
  }
}

void add_score_to_scoremanager(ScoreManager * sm, unsigned long points, char * name, char * finish_date) {
  if(sm == NULL) {
    return;
  }

  add_scoreobj_to_scoremanager(sm, create_score(points, name, finish_date));

  sort_scores(sm);
}

void display_highscores(ScoreManager *sm, int x, int y, char * font, int y_spacing){
  if(sm == NULL) {
    return;
  }

  if(sm->n_scores == 0) {
    //If there are no scores yet, display that on the screen
    string_to_screen("No scores were added yet...", font, x, y);
    y += y_spacing;
    string_to_screen("Try playing the game more", font, x, y);
    return;
  }

  const unsigned int max_scores_to_display = 5;
  //The number of scores that are going to be displayed is given by the minimum between the scores we can display and the maximum number of scores that we want to display
  unsigned int n_scores_to_display = MIN_VAL(sm->n_scores, max_scores_to_display);

  int i;
  for(i = 0; i < n_scores_to_display; i++) {
    char * score_str = score_to_string(sm->scores[i]);
    if(score_str == NULL) {
      continue;
    }

    //Creating a new char * to prepend "i: " (length: 3 (this string) + strlen of the original + 1 for null terminator + 2 for safety)
    char * temp_str = malloc((3 + strlen(score_str) + 1 + 2) * sizeof *temp_str);
    if(temp_str == NULL) {
      free(score_str);
      continue;
    }

    //Creating the string to print to screen
    sprintf(temp_str, "%u: %s", i + 1, score_str);

    string_to_screen(temp_str, font, x, y);

    //Freeing the strings and incrementing y according the the passed spacing
    free(score_str);
    free(temp_str);
    y += y_spacing;
  }
}

unsigned long scoremanager_get_highest_score (ScoreManager * sm) {
  if(sm == NULL || sm->n_scores == 0) {
    return 0;
  }

  return sm->scores[0]->points;
}
