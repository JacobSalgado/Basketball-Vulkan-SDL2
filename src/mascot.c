#include "simple_logger.h"

#include "player.h"
#include "basketball.h"

#include "collision.h"

#include "mascot.h"

Entity* mascot_new()
{
    Entity* self;
    self = entity_new();

    if (!self)
    {
        slog("failed to create new mascot entity");
        return NULL;
    }

    MascotData* mascotData;
    mascotData = gfc_allocate_array(sizeof(MascotData), 1);
    if (!mascotData)
    {
        slog("failed to allocate space for mascotData!");
        return NULL;
    }
    mascotData->state = MASCOT_WAITING;
    mascotData->timer = 10.0f;

    // data from court model so mascot knows how big court is and where to start
    GFC_Box court = collision_get_court();
    //float inset = 50.0f; // so dino isn't fully on the edge of court

    mascotData->startPosition = gfc_vector3d(/*court.x + */100, /*court.y*/ - 170, 30);
    mascotData->endPosition = gfc_vector3d(/*court.x + court.w*/ - 100, /*court.y + court.h*/ + 170, 30);

    self->data = mascotData;

    self->type = ENTITY_TYPE_HAZARD;
    self->model = model_load("models/dino.model");
    self->parent = NULL;

    self->localPosition = mascotData->startPosition;
    self->localRotation = gfc_vector3d(0,0,130);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(5,5,5);

    self->hidden = 0;

    self->color = GFC_COLOR_WHITE;

    self->think = mascot_think;
    self->update = mascot_update;
    self->free = mascot_free;

    slog("mascot(agumon) created!!");

    return self;
}

void mascot_think(Entity* self)
{
    if (!self) return;

    MascotData* mascotData;
    if (!self || !self->data) return;
    mascotData = (MascotData*) self->data;

    Entity* player = player_get();
    if (mascotData->state != MASCOT_CHARGING || mascotData->hitPlayer || !player) return;
    if (!check_entity_collision(self, player)) return;
    // TODO: invincibility check here later

    mascotData->hitPlayer = 1;
    player_knockdown(player);
    if (player_has_ball(player))
    {
        ball_detach(get_game_ball());
    }

}

void mascot_update(Entity* self, float deltaTime)
{
    if (!self) return;

    MascotData* mascotData;
    if (!self || !self->data) return;
    mascotData = (MascotData*)self->data;

    //slog("position x:%f y:%f", self->localPosition.x, self->localPosition.y);

    switch (mascotData->state)
    {
        case MASCOT_WAITING:
        {
            mascotData->timer -= deltaTime;
            if (mascotData->timer <= 0)
            {
                mascot_begin_charge(self, mascotData);
            }
            break;
        }
        case MASCOT_CHARGING:
        {
            float steps = 15.0f * deltaTime;
            if (steps >= mascotData->distanceLeft)
            {
                GFC_Vector3D newEndPosition = mascotData->startPosition;
                self->localPosition = mascotData->endPosition;
                mascotData->startPosition = mascotData->endPosition;
                mascotData->endPosition = newEndPosition;

                mascotData->state = MASCOT_WAITING;
                mascotData->timer = 10.0f;
            }
            else
            {
                self->localPosition.x += mascotData->direction.x * steps;
                self->localPosition.y += mascotData->direction.y * steps;
                mascotData->distanceLeft -= steps;
            }
        }
    }
}

void mascot_free(Entity* self)
{
    if (!self) return;

}

void mascot_begin_charge(Entity* self, MascotData* data)
{
    GFC_Vector3D distanceToTravel;
    gfc_vector3d_sub (distanceToTravel, data->endPosition, data->startPosition); // end pos minus start pos =  total distance
    data->distanceLeft = gfc_vector3d_magnitude(distanceToTravel);
    gfc_vector3d_normalize(&distanceToTravel);
    data->direction = distanceToTravel;

    //self->localRotation.z = atan2f(distanceToTravel.y, distanceToTravel.x);
    self->localRotation.z = -self->localRotation.z;
    data->hitPlayer = 0;
    data->state = MASCOT_CHARGING;
}
