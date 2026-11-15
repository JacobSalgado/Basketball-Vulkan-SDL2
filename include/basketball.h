#ifndef __BASKETBALL_H__
#define __BASKETBALL_H__

#include "entity.h"

typedef enum
{
    LOOSE,
    HELD,
    DRIBBLING,
    AIR,
    SCORED
} BasketballState;

typedef struct BallData_S
{
    BasketballState     state;          // current action of the basketball
    Entity*             owner;          // which player currently has the ball
    float               radius;         // radius of the ball
    float               restitution;    // how much energy an object keeps after a collision
    float               handHeight;     // y-axis offset for where the ball should be placed
    float               pushSpeed;      // force player pushes down to dribble the ball
    float               stillTimer;     // checks when the player is stopped or idle
    Uint8               wantsHold;      // stores when to pick up the ball
    GFC_Vector3D        launchPosition;
    GFC_Vector3D        launchVelocity;
    float               airTime;
    float               backspin;
} BallData;

/**
 * @brief allocates space for a new basketball entity
 * @return pointer to basketball entity, NULL if error
 */
Entity* basketball_new();

/**
 * @brief actions the basketball should follow
 * @param self the basketball
 */
void basketball_think(Entity* self);

/**
 * @brief the basketball to update
 * @param self the basketball
 */
void basketball_update(Entity* self, float deltaTime);

/**
 * @brief the basketball to free
 * @param self the basketball
 */
void basketball_free(Entity* self);

void ball_attach(Entity* ball, Entity* player);

void ball_detach(Entity* ball);

// TODO: Change to GFC_Sphere eventually
/**
 * @brief function for basketball collision box
 */
GFC_Box basketball_collision_box(Entity* self);

void ball_start_dribble();

Entity* ball_get_owner(Entity* owner);

Entity* get_game_ball();

void ball_gather(Entity *ball);

void ball_jumpshot(Entity* ball, GFC_Vector3D targetPosition);

#endif
