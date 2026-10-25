#include <SDL_events.h>

#include "simple_logger.h"

#include "player.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

int moveSpeed = 5;

Entity* player_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create a new entity");
        return NULL;
    }

    self->type = ENTITY_TYPE_PLAYER;
    self->model = model_load("models/dino.model");
    self->position = gfc_vector3d(0,0,0);
    self->rotation = gfc_vector3d(0,90,0);

    self->color = GFC_COLOR_WHITE;

    self->think = player_think;
    self->update = player_update;
    self->free = player_free;

    slog("player created!!");
    return self;
}

void player_think(Entity* self)
{
    if (!self) return;
}

void player_update(Entity* self)
{
    if (!self) return;
}

void player_free(Entity* self)
{
    if (!self) return;
}

/*void player_input(Entity* self, SDL_Event* event)
{
    if (!self || event) return;

    if (event->type == SDL_KEYDOWN && event->key.repeat == 0)
    {
        case SDLK_UP:
            self->position.x += moveSpeed; break;
    }
}*/
