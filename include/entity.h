#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_types.h"
#include "gfc_vector.h"

#include "model.h"

typedef struct Entity_S Entity;

typedef enum
{
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_OPPONENT,
    ENTITY_TYPE_BASKETBALL
} EntityType;

typedef struct Entity_S
{
    EntityType      type;
    Uint8           _refCount;
    Model*          model;
    Entity*         parent;         // note to self: be careful when freeing an entity cause a parent pointer might dangle
    GFC_Vector3D    localPosition;
    GFC_Vector3D    localRotation;
    GFC_Vector3D    localScale;
    GFC_Vector3D    worldPosition;
    GFC_Vector3D    worldRotation;
    GFC_Vector3D    worldScale;
    GFC_Vector3D    velocity;
    GFC_Color       color;

    void (*think)(struct Entity_S* self); /*<function to call to make decisions*/
    void (*update)(struct Entity_S* self); /*function to call to execute those decisions*/
    void (*free) (struct Entity_S* self); /*clean upp any custom allocated data*/
    void *data;								/* for ad hoc addition data for the entity */
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
 * @brief draws the entity to the screen
 * @param self the entity to ddraw
 */
void entity_draw(Entity* self);

/**
 * @brief draws all the active entities to the screen
 */
//void entity_system_draw()

/**
 * @brief run the think functions for all active entities
 */
void entity_system_think();

/**
 * @brief run the update functions for all active entities
 */
void entity_system_update();

/**
 * @brief loads in all active entities into the game
 */
void entity_system_load();

/**
 * @brief draws in all active entities into the game
 */
void entity_system_draw();



#endif
