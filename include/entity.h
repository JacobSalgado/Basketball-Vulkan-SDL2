#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_types.h"

typedef enum
{
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_OPPONENT
} EntityType;

typedef struct Entity_S
{
    EntityType      type;
    Uint8           _refCount;
    Model*          model;
    GFC_Vector2D    position;
    float           rotation
    GFC_Vector2D    scale;
    GFC_Vector2D    velocity;
} Entity;

/**
 * @brief initialize entity management system and queues up cleanup on exit
 * @param max the maximum num. of entities that can exist at the same time
 */
void entity_system_init(Uint32 max);

/**
 * @brief clean up all active entities
 */
void entity_clear_all();

/**
 * @brief get a pointer to a free entity
 * @return NULL if out of entities, a pointer to a blank entity otherwise
 */
Entity* entity_new();

/**
 * @brief clean up an entity, and free it's spot for future use
 * @param self the entity to free
 * @note do not use the memory address again after calling this
 */
void entity_free(Entity* self);

/**
 * @brief run the think functions for all active entities
 */
void entity_system_think();

/**
 * @brief run the update functions for all active entities
 */
void entity_system_update();



#endif
