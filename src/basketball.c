#include "simple_logger.h"

#include "basketball.h"

Entity* basketball_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new basketball entity");
        return NULL;
    }

    self->type = ENTITY_TYPE_BASKETBALL;
    self->model = model_load("models/basketball.model");

    self->position = gfc_vector3d(12,0,0);
    self->rotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->scale = gfc_vector3d(10,10,10);

    self->color = GFC_COLOR_WHITE;

    self->think = basketball_think;
    self->update = basketball_update;
    self->free = basketball_free;

    slog("basketball created!!");

    return self;
}

void basketball_think(Entity* self)
{
    if (!self) return;
}

void basketball_update(Entity* self)
{
    if (!self) return;
}

void basketball_free(Entity* self)
{
    if (!self) return;
}
