#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

#include "powerup.h"

typedef enum
{
    IDLE,
    MOVING,
    JUMPING,
    STUNNED
} PlayerState;

typedef struct PlayerData_S
{
    PlayerState             state;          // current action of the player
    PowerupEffectType       activePowerupEffect;
    float                   powerupEffectTimer; // how long the po
} PlayerData;

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

static Uint8 player_has_ball(Entity* self);

Entity* player_get();

void player_set_powerup(Entity* self, PowerupEffectType type, float duration);

Uint8 player_has_powerup(Entity* self, PowerupEffectType type);

void player_start_dunk(Entity* self, GFC_Vector3D rimPosition);

#endif
