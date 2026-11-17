#include <SDL.h>

#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "basketball.h"
#include "backboard.h"
#include "collision.h"

void player_think(Entity* self);
void player_update(Entity* self, float deltaTime);
void player_free(Entity* self);

static float moveSpeed = 5; // acceleration per frame

Entity* player_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create a new entity");
        return NULL;
    }
    PlayerData* playerData;

    playerData = gfc_allocate_array(sizeof(PlayerData), 1);
    if (!playerData)
    {
        slog("failed to allocate space for playerData!");
        return NULL;
    }
    playerData->state = IDLE;

    self->type = ENTITY_TYPE_PLAYER;
    self->model = model_load("models/player.model");
    self->parent = NULL;

    // has no parent entity so world and local positions are the same thing for the player
    self->localPosition = gfc_vector3d(0,0,15);
    self->localRotation = gfc_vector3d(0,90,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(2.5,2.5,2.5);

    self->color = GFC_COLOR_WHITE;

    self->hidden = 0;

    self->think = player_think;
    self->update = player_update;
    self->free = player_free;

    slog("player created!!");
    return self;
}

void player_think(Entity* self)
{
    if (!self) return;

    player_input(self);
}

void player_update(Entity* self, float deltaTime)
{
    //slog("position %f %f", self->position.x, self->position.y);
    if (!self) return;

    self->localPosition.x += self->velocity.x * deltaTime;
    self->localPosition.y += self->velocity.y * deltaTime;
    self->localPosition.z += self->velocity.z * deltaTime;

    self->velocity.x *= FRICTION;
    self->velocity.y *= FRICTION;

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
    if (gfc_input_command_down("forward")) // -y axis
        self->velocity.y -= moveSpeed;
    if (gfc_input_command_down("backward")) // +y axis
        self->velocity.y += moveSpeed;
    if (gfc_input_command_down("leftward")) // +x axis
        self->velocity.x += moveSpeed;
    if (gfc_input_command_down("rightward")) // -x axis
        self->velocity.x -= moveSpeed;

    // jump
    if (self->onGround && gfc_input_command_held("jump"))
    {
        self->velocity.z = JUMP_VELOCITY;
        self->onGround = 0;
        if (player_has_ball(self))
        {
            //self->state;
            ball_gather(get_game_ball());
        }
    }
    if (!self->onGround && player_has_ball(self) && gfc_input_command_pressed("shoot"))
    {
        ball_jumpshot(get_game_ball(), backboard_get_rim_position());
    }
}

static Uint8 player_has_ball(Entity* self)
{
    return ball_get_owner(get_game_ball()) == self;
}
