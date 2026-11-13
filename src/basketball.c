#include "simple_logger.h"

#include "basketball.h"

void basketball_think(Entity* self);
void basketball_update(Entity* self);
void basketball_free(Entity* self);

Entity* basketball_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new basketball entity");
        return NULL;
    }

    BallData* ballData;
    ballData = gfc_allocate_array(sizeof(BallData), 1);
    if (!ballData)
    {
        slog("failed to allocate space for balldata!");
        return NULL;
    }

    ballData->state = LOOSE;
    ballData->owner = NULL;
    ballData->radius = 0.12f;
    ballData->restitution = 0.82f;
    ballData->handHeight = 1.0f;

    self->data = ballData;

    self->type = ENTITY_TYPE_BASKETBALL;
    self->model = model_load("models/basketball.model");
    self->parent = NULL;

    self->localPosition = gfc_vector3d(50,15,15);
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
    BallData* ballData;
    if (!self || !self->data) return;
    ballData = (BallData*)self->data;

    if (!self->parent)
    {
        gfc_vector3d_add(self->localPosition, self->localPosition, self->velocity);
    }

    //slog("basketball worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("basketball localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);

    switch (ballData->state)
    {
        case LOOSE:
            //TODO: ball physics and bouncing and floor
            break;
        case HELD:
            //TODO: snap to player owner's hand
            break;
        case DRIBBLING:
            //TODO: follow player holding the ball as well as controlling bounce physics on Z
            break;
        case AIR:
            // TODO: flight physics and how it's going to land on rim or bounce off different surfaces
            break;
        case SCORED:
            // TODO: net timer and inbounding the ball
            break;
    }
}

void basketball_free(Entity* self)
{
    if (!self) return;
    if (!self->data) return;
    free(self->data);
    self->data = NULL;
}

void ball_attach(Entity* ball, Entity* player)
{
    BallData* ballData;
    if (!ball || !player || !ballData ) return;
    ballData = (BallData*)ball->data;

    ball->parent = player;
    ball->localPosition = gfc_vector3d(12, -5, 0);
    ball->velocity = gfc_vector3d(0,0,0);
    ballData->state = HELD;
    ballData->owner = player;
    slog("basketball attached to player!!");
}

void ball_detach(Entity* ball)
{
    BallData* ballData;
    if (!ball || !ballData) return;
    ballData = (BallData*)ball->data;

    ball->localPosition = ball->worldPosition;
    ball->parent = NULL;
    ballData->state = LOOSE;
    ballData->owner = NULL;
}
