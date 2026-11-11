#include <SDL.h>

#include "simple_logger.h"

#include "player.h"
#include "basketball.h"

void player_think(Entity* self);
void player_update(Entity* self);
void player_free(Entity* self);

static float moveSpeed = 1; // acceleration per frame
static float maxSpeed = 1.0;   // top speed player can reach
static float friction = 0.85;


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
    self->model = model_load("models/player.model");
    self->parent = NULL;

    self->localPosition = gfc_vector3d(0,0,7);
    self->localRotation = gfc_vector3d(0,90,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(2.5,2.5,2.5);

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
    unsigned char* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_UP]) self->velocity.y -= moveSpeed;
    if (keys[SDL_SCANCODE_DOWN]) self->velocity.y += moveSpeed;
    if (keys[SDL_SCANCODE_LEFT]) self->velocity.x += moveSpeed;
    if (keys[SDL_SCANCODE_RIGHT]) self->velocity.x -= moveSpeed;
}

void player_update(Entity* self)
{
    //slog("position %f %f", self->position.x, self->position.y);
    if (!self) return;

    self->localPosition.x += self->velocity.x;
    self->localPosition.y += self->velocity.y;
    self->localPosition.z += self->velocity.z;

    self->velocity.x *= friction;
    self->velocity.y *= friction;

    slog("player worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    slog("player localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);
}

void player_free(Entity* self)
{
    if (!self) return;
}

void player_input(Entity* self, SDL_Event* event)
{
    if (!self || !event) return;

    if (event->type == SDL_KEYDOWN && event->key.repeat == 0)
    {
        // movement handling
        switch (event->key.keysym.sym)
        {
            case SDLK_UP:
                slog("POsition changing %f", self->localPosition.x);
                self->localPosition.x += moveSpeed; break;
            case SDLK_DOWN:
                self->localPosition.x += moveSpeed; break;
        }
    }
}
