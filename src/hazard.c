#include "simple_logger.h"

#include "player.h"

#include "collision.h"

#include "hazard.h"

#define MAX_HAZARDS_ON_COURT 10

static Entity* _hazards[MAX_HAZARDS_ON_COURT];
static int _hazardCount = 0;

static const Hazard _hazardManager[MAX_HAZARDS_ON_COURT] =
{
    [HAZARD_DEADSPOT] = {"models/hazard.model", 30.0f},
    [HAZARD_DIZZY] = {"models/hazard.model", 10.0f},
};

Entity* hazard_new(HazardType type, GFC_Vector3D position)
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new hazard entity");
        return NULL;
    }

    HazardData* hazardData;
    hazardData = gfc_allocate_array(sizeof(HazardData), 1);
    if (!hazardData)
    {
        slog("failed to allocate space for hazardData!");
        return NULL;
    }
    hazardData->type = type;
    hazardData->active = 1;

    self->data = hazardData;

    self->type = ENTITY_TYPE_HAZARD;
    self->model = model_load(_hazardManager[hazardData->type].model);
    self->parent = NULL;

    self->localPosition = position;
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(5,5,5);

    self->hidden = 0;

    self->color = GFC_COLOR_WHITE;

    self->think = hazard_think;
    self->update = hazard_update;
    self->free = hazard_free;

    if (_hazardCount < MAX_HAZARDS_ON_COURT)
        _hazards[_hazardCount++] = self;
    else
    {
        slog("NO MORE SPACE TO CREATE A NEW HAZARD");
        return NULL;
    }

    slog("hazard created!!");

    return self;
}

void hazard_think(Entity* self)
{
    HazardData* hazardData;
    Entity* player = player_get();
    if (!self || !self->data || !player) return;
    hazardData = (HazardData*)self->data;

    if (!hazardData->active || !check_entity_collision(player_get(), self)) return;
}

void hazard_update(Entity* self, float deltaTime)
{
    HazardData *hazardData;
    if (!self || !self->data) return;
    hazardData = (HazardData *)self->data;
}

void hazard_free(Entity* self)
{
    if (!self) return;
}

Uint8 hazard_at(GFC_Vector3D position, HazardType type)
{
    for (int i = 0; i < _hazardCount; i++)
    {
        HazardData* hazardData = (HazardData*)_hazards[i]->data;
        GFC_Box box;

        if (!hazardData || !hazardData->active || hazardData->type != type) continue;
        box = entity_box_collision(_hazards[i]);
        if (position.x >= box.x &&
            position.x <= box.x + box.w &&
            position.y >= box.y &&
            position.y <= box.y + box.h
        )
        {
            slog("player colliding with hazard!!");
            return 1;
        }

    }
    return 0;
}

float hazard_get_duration(HazardType type)
{
    if (type < 0 || type >= HAZARD_MAX) return 0.0f;
    return _hazardManager[type].duration;
}
