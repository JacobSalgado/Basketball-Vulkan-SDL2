#include <SDL.h>

#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "basketball.h"
#include "collision.h"

void player_think(Entity* self);
void player_update(Entity* self, float deltaTime);
void player_free(Entity* self);

static float moveSpeed = 1; // acceleration per frame
static float maxSpeed = 1.0;   // top speed player can reach
static float friction = 0.85;
static float jumpForce = 10.0;

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

    // has no parent entity so world and local positions are the same thing for the player
    self->localPosition = gfc_vector3d(0,0,15);
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

    player_input(self);
}

void player_update(Entity* self, float deltaTime)
{
    //slog("position %f %f", self->position.x, self->position.y);
    if (!self) return;

    self->localPosition.x += self->velocity.x;
    self->localPosition.y += self->velocity.y;
    self->localPosition.z += self->velocity.z;

    self->velocity.x *= friction;
    self->velocity.y *= friction;

    //slog("player worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("player localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);
}

void player_free(Entity* self)
{
    if (!self) return;
}

void player_input(Entity* self)
{
    // movement
    if (gfc_input_command_down("forward"))
        self->velocity.y -= moveSpeed;
    if (gfc_input_command_down("backward"))
        self->velocity.y += moveSpeed;
    if (gfc_input_command_down("leftward"))
        self->velocity.x += moveSpeed;
    if (gfc_input_command_down("rightward"))
        self->velocity.x -= moveSpeed;

    // jump
    if (self->onGround && gfc_input_command_held("jump"))
    {
        self->velocity.z = JUMP_HEIGHT;
        self->onGround = 0;
    }
}
