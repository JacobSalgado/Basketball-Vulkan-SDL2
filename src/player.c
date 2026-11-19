#include <SDL.h>

#include "simple_logger.h"

#include "gfc_input.h"

#include "player.h"
#include "basketball.h"
#include "backboard.h"
#include "collision.h"
#include "hazard.h"

void player_think(Entity* self);
void player_update(Entity* self, float deltaTime);
void player_free(Entity* self);

static Entity* _player = NULL;

static float moveSpeed = 5; //

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
    playerData->activePowerupEffect = POWERUP_NONE;

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

    self->data = playerData;

    self->think = player_think;
    self->update = player_update;
    self->free = player_free;

    _player = self;

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
    PlayerData* playerData;
    if (!self || !self->data) return;
    playerData = (PlayerData*) self->data;
    //slog("position x:%f y:%f", self->localPosition.x, self->localPosition.y);

    self->localPosition.x += self->velocity.x * deltaTime;
    self->localPosition.y += self->velocity.y * deltaTime;
    self->localPosition.z += self->velocity.z * deltaTime;

    self->velocity.x *= FRICTION;
    self->velocity.y *= FRICTION;

    // powerup timers
    if (playerData->powerupEffectTimer > 0)
    {
        playerData->powerupEffectTimer -= deltaTime;
        if (playerData->powerupEffectTimer <= 0)
        {
            slog("powerup expired");
            playerData->activePowerupEffect = POWERUP_NONE;
            playerData->powerupEffectTimer  = 0;
        }
    }

    if (hazard_at(self->localPosition, HAZARD_DIZZY)) //TODO: need to add future flag when adding invincibility powerup
    {
        playerData->dizzyTimer = hazard_get_duration(HAZARD_DIZZY);
    }

    // dizzy hazard timer timer
    if (playerData->dizzyTimer > 0)
        playerData->dizzyTimer -= deltaTime;

    // knockdown info
    if (playerData->knockdownTimer > 0)
        playerData->knockdownTimer -= deltaTime;

    //slog("player worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("player localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);
}

void player_free(Entity* self)
{
    if (!self) return;
}

void player_input(Entity* self)
{
    float direction;
    PlayerData* playerData;
    if (!self || !self->data) return;
    playerData = (PlayerData*) self->data;

    // for when/if player is knocked down by agumon
    if (playerData->knockdownTimer > 0) return;
    else self->localRotation.x = 0; // put player back in regular position

    // for when/if player is dizzy
    if (playerData->dizzyTimer > 0)
        direction = -1.0f;
    else
        direction = 1.0f;

    // movement
    if (gfc_input_command_down("forward")) // -y axis
        self->velocity.y -= moveSpeed * direction;
    if (gfc_input_command_down("backward")) // +y axis
        self->velocity.y += moveSpeed * direction;
    if (gfc_input_command_down("leftward")) // +x axis
        self->velocity.x += moveSpeed * direction;;
    if (gfc_input_command_down("rightward")) // -x axis
        self->velocity.x -= moveSpeed * direction;;

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

Uint8 player_has_ball(Entity* self)
{
    return ball_get_owner(get_game_ball()) == self;
}

Entity* player_get()
{
    return _player;
}

void player_set_powerup(Entity* self, PowerupEffectType type, float duration)
{
    slog("player set powerup called!");
    PlayerData* playerData;
    if (!self || !self->data) return;
    playerData = (PlayerData*) self->data;

    playerData->activePowerupEffect = type;
    playerData->powerupEffectTimer = duration;
    slog("powerup %d active for %.1f seconds", type, duration);

    if (type == POWERUP_BONUS_POINTS)
    {
        playerData->bonusPoints = 3 + rand() % 3; // 3, 4, or 5
        slog("bonus points! next made shot is worth %d", playerData->bonusPoints);
    }
    if (hazard_at(self->localPosition, HAZARD_DIZZY)) //TODO: need to add future flag when adding invincibility powerup
    {
        playerData->dizzyTimer = hazard_get_duration(HAZARD_DIZZY);
    }
}

Uint8 player_has_powerup(Entity* self, PowerupEffectType type)
{
    PlayerData* playerData;
    if (!self || !self->data) return 0;
    playerData = (PlayerData*)self->data;
    return (playerData->activePowerupEffect == type && playerData->powerupEffectTimer > 0);
}

int player_give_points(Entity* player)
{
    PlayerData* playerData;
    int points = 2;
    if (!player || !player->data) return points;
    playerData = (PlayerData*)player->data;

    if (player_has_powerup(player, POWERUP_BONUS_POINTS))
    {
        points = playerData->bonusPoints;
        playerData->activePowerupEffect = POWERUP_NONE;
        playerData->powerupEffectTimer = 0;
        playerData->bonusPoints = 0;
    }
    return points;
}

void player_knockdown(Entity* self)
{
    PlayerData* playerData;
    if (!self || !self->data) return;
    playerData = (PlayerData*)self->data;
    playerData->knockdownTimer = 2.0f;
    self->localRotation.x = 45;
    self->velocity.x = 0;
    self->velocity.y = 0;
}
