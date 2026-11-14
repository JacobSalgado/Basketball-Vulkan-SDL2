#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef enum
{
    IDLE, RUN, DRIBBLE, JUMPSHOT, DUNK, STUNNED
} PlayerState;

/**
 * @brief allocates space for a new player entity
 * @return pointer to player, NULL if error
 */
Entity* player_new();

/**
 * @brief actions the player should do
 * @param self the player
 */
void player_think(Entity* self);

/**
 * @brief the player to update
 * @param self the player
 */
void player_update(Entity* self, float deltaTime);

/**
 * @brief the player to free
 * @param self the player
 */
void player_free(Entity* self);

/**
 * @brief handles any player input actions
 * @param self the player you are controlling
 */
void player_input(Entity* self);

/**
 * @brief function for player collision box
 */
GFC_Box player_collision_box(Entity* self);

#endif
