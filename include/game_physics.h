#ifndef __GAME_PHYSICS_H__
#define __GAME_PHYSICS_H__

#include "gfc_vector.h"

// game-related
#define GAME_FPS 30
#define GAME_DELTA_TIME (1.0f / GAME_FPS)
#define UNITS_PER_METER (400 / 28.65f) // 400 is the length of the court, and 28.65m is the length of an NBA court
#define GRAVITY (9.8f * UNITS_PER_METER)

#define FRICTION 0.9

// Basketball stuff
#define SETTLE_SPEED 9.0f
#define SHOT_TIME_BASE 0.7f // minimum seconds of flight
#define SHOT_TIME_PER_UNIT 0.005f // extra seconds for each unit of distance
#define MAX_AIM_ERROR 3
#define MAKE_THRESHOLD 0.7 // release quality needed in order to make a basket
#define BACKSPIN_RATE 2.5
//#define RIM_RADIUS (0.23 * UNITS_PER_METER)
#define RIM_OFFSET gfc_vector3d(0, 55, 32.6f)
#define PICKUP_COOLDOWN 2.0f

// player stuff
#define JUMP_HEIGHT (3 * UNITS_PER_METER)
#define JUMP_VELOCITY (sqrtf(2.0f * GRAVITY * JUMP_HEIGHT)) // jump speed = square root of 2gh
#define MOVE_THRESHOLD 5
#define HOLD_DELAY 0.25
#define BALL_HOLD_OFFSET gfc_vector3d(12,15,12)
#define BALL_DRIBBLE_OFFSET 12
#define BALL_SHOOT_OFFSET gfc_vector3d(0,15,20)

// handling score stuff
#define SCORED_HIDE_TIME 0.75f
#define SCORED_RESPAWN_TIME 1.5f
#define BALL_RESPAWN_POSITION gfc_vector3d(50,15,40)

float horizontal_distance(GFC_Vector3D a, GFC_Vector3D b);

#endif
