#include "simple_logger.h"

#include "entity.h"
#include "player.h"

#include "collision.h"

#include "powerup.h"

static const Powerup _powerupManager[POWERUP_MAX] =
{
    [POWERUP_HOT_HAND] = {"models/powerup.model", 10.0f, 15.0f},
};

Entity* powerup_new(PowerupEffectType type, GFC_Vector3D position)
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new powerup entity");
        return NULL;
    }

    PowerupData* powerupData;
    powerupData = gfc_allocate_array(sizeof(PowerupData), 1);
    if (!powerupData)
    {
        slog("failed to allocate space for powerupData!");
        return NULL;
    }
    powerupData->type = type;
    powerupData->active = 1;
    powerupData->respawnTimer = _powerupManager[powerupData->type].respawnTime;

    self->data = powerupData;

    self->type = ENTITY_TYPE_POWERUP;
    self->model = model_load(_powerupManager[powerupData->type].model);
    self->parent = NULL;

    self->localPosition = position;
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

    self->hidden = 0;

    self->color = GFC_COLOR_WHITE;

    self->think = powerup_think;
    self->update = powerup_update;
    self->free = powerup_free;

    slog("powerup created!!");

    return self;
}

void powerup_think(Entity* self)
{
    PowerupData* powerupData;
    Entity* player = player_get();
    if (!self || !self->data || !player) return;
    powerupData = (PowerupData*)self->data;

    if (!powerupData->active || !check_entity_collision(player_get(), self)) return;

    player_set_powerup(player, powerupData->type, _powerupManager[powerupData->type].duration);
    powerupData->active = 0;
    powerupData->respawnTimer = _powerupManager[powerupData->type].respawnTime;
    self->hidden = 1;
}

void powerup_update(Entity* self, float deltaTime)
{
    PowerupData *powerupData;
    if (!self || !self->data) return;
    powerupData = (PowerupData *)self->data;

    if (!powerupData->active)
    {
        powerupData->respawnTimer -= deltaTime;
        if (powerupData->respawnTimer <= 0)
        {
            powerupData->active = 1;
            self->hidden = 0;
        }
        return;
    }
    self->localRotation.z += 2.0f * deltaTime; // slow spin motion
}

void powerup_free(Entity* self)
{
    if (!self) return;
}

