#ifndef __GAME_PHYSICS_H__
#define __GAME_PHYSICS_H__

// game-related
#define GAME_FPS 30
#define GAME_DELTA_TIME 1.0f / GAME_FPS
#define UNITS_PER_METER 400 / 28.65f // 400 is the length of the court, and 28.65m is the length of an NBA court
#define GRAVITY 9.8f * UNITS_PER_METER

#define FRICTION 0.9


// Basketball stuff
#define SETTLE_SPEED 9.0f

// player stuff
#define JUMP_HEIGHT 2 * UNITS_PER_METER
#define MOVE_THRESHOLD 5
#define HOLD_DELAY 0.25
#define BALL_HOLD_OFFSET 12
#define BALL_DRIBBLE_OFFSET 12

#endif
