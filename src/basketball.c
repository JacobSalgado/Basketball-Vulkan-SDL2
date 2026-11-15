#include "simple_logger.h"

#include "basketball.h"

#include "collision.h"

void basketball_think(Entity* self);
void basketball_update(Entity* self, float deltaTime);
void basketball_free(Entity* self);

// help function since I need it ball_start_dribble not just update
static float ball_floor_local_z(Entity* ball, BallData* ballData)
{
    if (!ball || !ballData) return 0;

    float ownerZ;
    if (ballData->owner)
    {
        ownerZ = ballData->owner->localPosition.z;
    }
    else
    {
        ownerZ = 0.0f;
    }
    return collision_get_floor_z() + ballData->radius - ownerZ;
}

// help function since I need it ball_start_dribble not just update
static float ball_hand_local_z(Entity* ball, BallData* ballData)
{
    if (!ball || !ballData) return 0;

    float ownerZ;
    if (ballData->owner)
    {
        ownerZ = ballData->owner->localPosition.z;
    }
    else
    {
        ownerZ = 0.0f;
    }
    return collision_get_floor_z() + ballData->handHeight - ownerZ;
}


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
    ballData->restitution = 0.88f;
    ballData->handHeight = 1.0f * UNITS_PER_METER;
    ballData->stillTimer = 0;
    ballData->pushSpeed = 3;

    self->data = ballData;

    self->type = ENTITY_TYPE_BASKETBALL;
    self->model = model_load("models/basketball.model");
    self->parent = NULL;

    self->localPosition = gfc_vector3d(50,15,40);
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

    if (self->model && self->model->mesh)
        ballData->radius = self->model->mesh->bounds.w * 0.5f * self->localScale.x;

    self->color = GFC_COLOR_WHITE;

    self->think = basketball_think;
    self->update = basketball_update;
    self->free = basketball_free;

    slog("basketball created!!");

    return self;
}

void basketball_think(Entity* self)
{
    BallData* ballData;
    if (!self || !self->data) return;
    ballData = (BallData*)self->data;

    if (ballData->state == HELD && ballData->owner != NULL)
    {
        if ((gfc_vector2d_magnitude_squared(gfc_vector2d(ballData->owner->velocity.x, ballData->owner->velocity.y))) > MOVE_THRESHOLD)
            ball_start_dribble(self);
    }

    if (ballData->state == DRIBBLING && ballData->owner != NULL)
    {
        if ((gfc_vector2d_magnitude_squared(gfc_vector2d(ballData->owner->velocity.x, ballData->owner->velocity.y))) > MOVE_THRESHOLD)
        {
            ballData->stillTimer = 0;
            ballData->wantsHold = 0;
        }
        else
        {
            ballData->stillTimer += GAME_DELTA_TIME;
            if (ballData->stillTimer >= HOLD_DELAY)
                ballData->wantsHold = 1;
        }
    }
}

void basketball_update(Entity* self, float deltaTime)
{
    BallData* ballData;
    if (!self || !self->data) return;
    ballData = (BallData*)self->data;

    //slog("basketball worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("basketball localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);

    switch (ballData->state)
    {
        float floorZ = collision_get_floor_z() + ballData->radius;
        float ownerZ = ballData->owner->localPosition.z;
        float floorLocal = ball_floor_local_z(self, ballData);
        float handLocal = ball_hand_local_z(self, ballData);

        case LOOSE:
        {
            apply_gravity(self, deltaTime);

            self->localPosition.x += self->velocity.x * deltaTime;
            self->localPosition.y += self->velocity.y * deltaTime;
            self->localPosition.z += self->velocity.z * deltaTime;

            if (self->localPosition.z <= floorZ) // undos the ball from going straight through the floor
            {
                self->localPosition.z = floorZ;
                if (self->velocity.z < 0)
                {
                    self->velocity.z = -self->velocity.z * ballData->restitution;
                    self->velocity.x *= FRICTION;
                    self->velocity.y *= FRICTION;
                    if (fabsf(self->velocity.z) < SETTLE_SPEED) // slows ball down to let it sit and rest
                    {
                        self->velocity.z = 0;
                        self->onGround = 1;
                    }

                }
            }
            else
            {
                self->onGround = 0;
            }
            break;
        }
        case HELD:
        {
            self->localPosition = gfc_vector3d(12,12,15);
            self->velocity = gfc_vector3d(0,0,0);
        }
            //TODO: snap to player owner's hand. Hold at first but don't dribble until player starts moving
            break;
        case DRIBBLING:
        {
            self->localPosition.x = 12;
            self->localPosition.y = 5;
            self->velocity.z -= GRAVITY * deltaTime;
            self->localPosition.z += self->velocity.z * deltaTime;

            if ((self->localPosition.z <= floorLocal) && self->velocity.z < 0)
            {
                self->localPosition.z = floorLocal;
                self->velocity.z = -self->velocity.z * ballData->restitution;
            }
            if ((self->localPosition.z >= handLocal && self->velocity.z > 0))
            {
                self->localPosition.z = handLocal;
                self->velocity.z = -ballData->pushSpeed;
            }
        }
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
    self->data = NULL;
}

void ball_attach(Entity* ball, Entity* player)
{
    BallData* ballData;
    if (!ball || !player || !ball->data ) return;
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
    if (!ball || !ball->data) return;
    ballData = (BallData*)ball->data;

    ball->localPosition = ball->worldPosition;
    ball->parent = NULL;
    ballData->state = LOOSE;
    ballData->owner = NULL;
}

void ball_start_dribble(Entity* ball)
{
    BallData* ballData;
    if (!ball || !ball->data) return;
    ballData = (BallData*)ball->data;

    ballData->state = DRIBBLING;
    ball->localPosition.x = 12;
    ball->localPosition.y = 5;
    ball->localPosition.z = ball_hand_local_z(ball, ballData);
    ball->velocity.z = ballData->pushSpeed;

    // reset
    ballData->stillTimer = 0;
    ballData->wantsHold = 0;
}
