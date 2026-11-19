#ifndef __MASCOT_H__
#define __MASCOT_H__

#include "entity.h"

typedef enum
{
    MASCOT_WAITING,
    MASCOT_CHARGING
} MascotState;

typedef struct
{
    MascotState         state;
    float               timer;
    float               distanceLeft;
    GFC_Vector3D        startPosition;
    GFC_Vector3D        endPosition;
    GFC_Vector3D        direction;
    Uint8               hitPlayer;
} MascotData;

Entity* mascot_new();

void mascot_think(Entity* self);

/**
 * @brief the mascot to update
 * @param self the mascot
 */
void mascot_update(Entity* self, float deltaTime);

/**
 * @brief the mascot to free
 * @param self the mascot
 */
void mascot_free(Entity* self);

void mascot_begin_charge(Entity* self, MascotData* data);

#endif
