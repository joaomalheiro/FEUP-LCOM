#ifndef __SCOREMANAGER_H
#define __SCOREMANAGER_H

#include "score.h"

/** @defgroup scoremanager scoremanager
 * @{
 *
 * Functions and structs for Score management
 */

#define LINE_MAX_LENGTH  80
#define SCORES_TXT_LOCATION "/home/Robinix/scores/scores.txt"

typedef struct {
  Score ** scores;
  unsigned int n_scores;
} ScoreManager;

/**
 * @brief Creates a ScoreManager
 * @return Pointer to a valid ScoreManager or NULL in case of failure
 */
ScoreManager* create_scoremanager();

/**
 * @brief Destroys the passed ScoreManager
 * @param sm_ptr The ScoreManager to destroy
 */
void destroy_scoremanager(ScoreManager ** sm_ptr);

/**
 * @brief Writes the scores saved in the ScoreManager to the scores txt file
 * @param sm The ScoreManager whose scores to store
 */
void write_scores_to_scores_file(ScoreManager * sm);

/**
 * @brief Adds a score to the ScoreManger
 * @param sm          ScoreManager to add the score to
 * @param points      Points of the new score
 * @param name        Name of the new score owner
 * @param finish_date Finish date of the new score
 */
void add_score_to_scoremanager(ScoreManager * sm, unsigned long points, char * name, char * finish_date);

/**
 * @brief Displays the top 3 scores
 * @param sm        ScoreManager whose highscores to display
 * @param x         The x in the screen at which to display the scores
 * @param y         The y in the screen at which to display the scores
 * @param font      The font to use in the highscore display
 * @param y_spacing The y spacing between lines
 */
void display_highscores(ScoreManager *sm, int x, int y, char* font, int y_spacing);

/**
 * @brief Returns the points of the highest score in the ScoreManager Object
 * @param  sm The ScoreManager to get the highest score for
 * @return    Returns the highest score or 0 in case there is not one or the ScoreManager object is invalid
 */
unsigned long scoremanager_get_highest_score(ScoreManager * sm);

#endif /* __SCOREMANAGER_H */
