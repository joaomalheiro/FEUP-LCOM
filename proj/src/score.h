#ifndef __SCORE_H
#define __SCORE_H

/** @defgroup score score
 * @{
 *
 * Functions and structs for Score creation and operation
 */

typedef struct {
  unsigned long points;
  char * name;
  char * finish_date;
} Score;

/**
 * @brief Creates a new Score based on the passed arguments
 * @param  points      The number of points that were scored
 * @param  name        The name of the player
 * @param  finish_date The finish date of the score
 * @return             Returns a pointer to a valid score or NULL if an error ocurred
 */
Score * create_score(unsigned long points, const char * name, const char * finish_date);

/**
 * @brief Creates a new Score based on a passed string
 * @param  s String to interpret, in the format <Points> <Name> <Date> in which these elements are space separated and date is YY/MM/DD
 * @return   Returns a pointer to a valid score or NULL if an error ocurred
 */
Score * create_score_from_string(const char * s);

/**
 * @brief Destroys the passed score
 * @param s_ptr Score to destroy
 */
void destroy_score(Score ** s_ptr);

/**
 * @brief Converts a score the a string following the format <Points> <Name> <Date>
 * @param  s_ptr The score to convert to string
 * @return       A pointer to a valid null-terminated string sequence representing the passed score, or NULL in case of failure
 */
char * score_to_string (Score * s_ptr);


#endif /* __SCORE_H */
