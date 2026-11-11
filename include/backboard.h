#ifndef __BACKBOARD_H__
#define __BACKBOARD_H__

#include "entity.h"

/**
 * @brief allocates space for a new backboard and hoop entity
 * @return pointer to backboard and hoop entity, NULL if error
 */
Entity* backboard_new();

/**
 * @brief actions the backboard should follow
 * @param self the basketball
 */
void backboard_think(Entity* self);

/**
 * @brief the backboard to update
 * @param self the backboard
 */
void backboard_update(Entity* self);

/**
 * @brief the backboard to free
 * @param self the backboard
 */
void backboard_free(Entity* self);

#endif
