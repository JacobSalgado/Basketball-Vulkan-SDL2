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
    self->parent = NULL;

    self->localPosition = gfc_vector3d(50,15,0);
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

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
    if (!self->parent)
    {
        gfc_vector3d_add(self->localPosition, self->localPosition, self->velocity);
    }
    //slog("basketball worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("basketball localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);
}

void basketball_free(Entity* self)
{
    if (!self) return;
}

void ball_attach(Entity* ball, Entity* player)
{
    if (!ball || !player) return;

    ball->parent = player;
    ball->localPosition = gfc_vector3d(12, -5, 0);
    slog("basketball attached to player!!");
}

void ball_detach(Entity* ball)
{
    if (!ball) return;
    ball->localPosition = ball->worldPosition;
    ball->parent = NULL;
}
