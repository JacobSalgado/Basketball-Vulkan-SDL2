#ifndef __HAZARD_H__
#define __HAZARD_H__

#include "entity.h"

typedef enum
{
    HAZARD_DEADSPOT,        // low restituion
    HAZARD_DIZZY,          // reverses player inputs so they're in a daze
    HAZARD_MASCOT,
    HAZARD_SLIPPERY,
    HAZARD_COLD,             // shooting from a specific spot raises the make threshold making it more difficult to make a shot
    HAZARD_MAX
} HazardType;

typedef struct Hazard_S
{
    const char          *model;     // model of the hazard
    float               duration;   // seconds the hazard lasts
} Hazard;

typedef struct
{
    HazardType              type;
    Uint8                   active;
} HazardData;

/**
 * @brief creates a brand new
 * @param type
 */
Entity* hazard_new(HazardType type, GFC_Vector3D position);

/**
 * @brief actions the hazard should follow
 * @param self the hazard
 */
void hazard_think(Entity* self);

/**
 * @brief the hazard to update
 * @param self the hazard
 */
void hazard_update(Entity* self, float deltaTime);

/**
 * @brief the hazard to free
 * @param self the hazard
 */
void hazard_free(Entity* self);

#endif
