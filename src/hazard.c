#include "simple_logger.h"

#include "player.h"

#include "collision.h"

#include "hazard.h"

static const Hazard _hazardManager[HAZARD_MAX] =
{
    [HAZARD_DEADSPOT] = {"models/hazard.model", 40.0f, 15.0f},
    [HAZARD_RUBBER] = {"models/hazard.model", -40.0f, 15.0f},
};

Entity* hazard_new(HazardType type, GFC_Vector3D position)
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new hazard entity");
        return NULL;
    }

    HazardData* hazardData;
    hazardData = gfc_allocate_array(sizeof(HazardData), 1);
    if (!hazardData)
    {
        slog("failed to allocate space for hazardData!");
        return NULL;
    }
    hazardData->type = type;
    hazardData->active = 1;

    self->data = hazardData;

    self->type = ENTITY_TYPE_HAZARD;
    self->model = model_load(_hazardManager[hazardData->type].model);
    self->parent = NULL;

    self->localPosition = position;
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(5,5,5);

    self->hidden = 0;

    self->color = GFC_COLOR_WHITE;

    self->think = hazard_think;
    self->update = hazard_update;
    self->free = hazard_free;

    slog("hazard created!!");

    return self;
}

void hazard_think(Entity* self)
{
    HazardData* hazardData;
    Entity* player = player_get();
    if (!self || !self->data || !player) return;
    hazardData = (HazardData*)self->data;

    if (!hazardData->active || !check_entity_collision(player_get(), self)) return;
}

void hazard_update(Entity* self, float deltaTime)
{
    HazardData *hazardData;
    if (!self || !self->data) return;
    hazardData = (HazardData *)self->data;
}

void hazard_free(Entity* self)
{
    if (!self) return;
}
