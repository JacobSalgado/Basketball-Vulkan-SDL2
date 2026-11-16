#ifndef __SCORE_H__
#define __SCORE_H__

/**
 * @brief adds points (2 or 3 depending on position) to current count
 * @param points num of points to add
 */
void score_add(int points);

/**
 * @return current score number
 */
int score_get();

/**
 * @brief resets the score after a round
 */
void score_reset();

#endif
