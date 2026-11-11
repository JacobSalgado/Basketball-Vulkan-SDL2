#ifndef __BASKETBALL_H__
#define __BASKETBALL_H__

typedef enum
{
    DRIBBLING,
    AIR,
    LOOSE
} BasketballState;

typedef enum {
    HIGH,
    LOW
} DribblingState;

#include "entity.h"

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
void basketball_update(Entity* self);

/**
 * @brief the basketball to free
 * @param self the basketball
 */
void basketball_free(Entity* self);

void ball_attach(Entity* ball, Entity* player);

void ball_detach(Entity* ball);

#endif
