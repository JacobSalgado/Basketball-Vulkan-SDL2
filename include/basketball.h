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
    BasketballState     state;
    Entity*             owner;
    float               radius;
    float               restitution;
    float               handHeight;
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

#endif
