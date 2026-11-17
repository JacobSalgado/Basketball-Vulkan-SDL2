#ifndef __POWERUP_H__
#define __POWERUP_H__

#include "entity.h"

typedef enum
{
    POWERUP_NONE = -1,      // no active powerup
    POWERUP_HOT_HAND,       // releaseQuality threshold is decreased to increase chance of making a shot
    POWERUP_SPEED,          // increase the speed of the player. maybe jump velocity?
    POWERUP_BONUS_POINTS,   // if picked up then 3 or more points
    POWERUP_INVINCIBLITY,   // cannot be shoved to ground and ball cannot be stolen
    POWERUP_MAX
} PowerupEffectType;

typedef struct Powerup_S
{
    const char          *model;
    float               duration; // seconds the powerup lasts
    float               respawnTime; // seconds before the pickup comes back
} Powerup;

typedef struct
{
    PowerupEffectType       type;
    Uint8                   active;
    float                   respawnTimer;
} PowerupData;


Entity* powerup_new(PowerupEffectType type, GFC_Vector3D position);

/**
 * @brief actions the powerup should follow
 * @param self the powerup
 */
void powerup_think(Entity* self);

/**
 * @brief the powerup to update
 * @param self the powerup
 */
void powerup_update(Entity* self, float deltaTime);

/**
 * @brief the powerup to free
 * @param self the powerup
 */
void powerup_free(Entity* self);

#endif
