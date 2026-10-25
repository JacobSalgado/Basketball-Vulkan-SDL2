#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

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
void player_update(Entity* self);

/**
 * @brief the player to free
 * @param self the player
 */
void player_free(Entity* self);

/**
 * @brief handles any player input actions
 * @param self the player
 * @param event the event pointer address
 */
//void player_input(Entity* self, SDL_Event* event)

#endif
