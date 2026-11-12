#include "simple_logger.h"

#include "backboard.h"

Entity* backboard_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new backboard entity");
        return NULL;
    }

    self->type = ENTITY_TYPE_BACKBOARD;
    self->model = model_load("models/backboard.model");

    self->localPosition = gfc_vector3d(16,0,0);
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

    self->color = GFC_COLOR_WHITE;

    self->think = backboard_think;
    self->update = backboard_update;
    self->free = backboard_free;

    slog("backboard created!!");

    return self;
}

void backboard_think(Entity* self)
{
    if (!self) return;
}

void backboard_update(Entity* self)
{
    if (!self) return;
}

void backboard_free(Entity* self)
{
    if (!self) return;
}
