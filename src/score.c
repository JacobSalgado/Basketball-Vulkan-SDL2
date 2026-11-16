#include "simple_logger.h"

#include "score.h"

static int _score = 0;

void score_add(int points)
{
    _score += points;
}

int score_get()
{
    return _score;
}

void score_reset()
{
    _score = 0;
}
