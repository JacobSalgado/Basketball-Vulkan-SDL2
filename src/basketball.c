#include "simple_logger.h"

#include "basketball.h"

#include "collision.h"

void basketball_think(Entity* self);
void basketball_update(Entity* self, float deltaTime);
void basketball_free(Entity* self);

static Entity* _basketball = NULL; // only one basketball in the game

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
    ballData->restitution = 0.85f; // very bouncy. if it equals 1, then that makes it a perfect bounce and no energy is lost
    ballData->handHeight = 1.0f * UNITS_PER_METER;
    ballData->stillTimer = 0;

    self->data = ballData;

    self->type = ENTITY_TYPE_BASKETBALL;
    self->model = model_load("models/basketball.model");
    self->parent = NULL;

    self->localPosition = gfc_vector3d(50,15,40);
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

    float fallHeight = ballData->handHeight - ballData->radius;
    float targetHeight = fallHeight * 1.05f;
    float e = ballData->restitution;
    ballData->pushSpeed = sqrtf(2.0f * GRAVITY * (targetHeight / (e * e) - fallHeight));

    if (self->model && self->model->mesh)
        ballData->radius = self->model->mesh->bounds.w * 0.5f * self->localScale.x;

    self->color = GFC_COLOR_WHITE;

    self->think = basketball_think;
    self->update = basketball_update;
    self->free = basketball_free;

    _basketball = self;

    slog("basketball created!!");
    //slog("radius %.2f handHeight %.2f pushSpeed %.2f", ballData->radius, ballData->handHeight, ballData->pushSpeed);

    return self;
}

void basketball_think(Entity* self)
{
    BallData* ballData;
    if (!self || !self->data) return;
    ballData = (BallData*)self->data;

    if (ballData->state == HELD && ballData->owner != NULL && ballData->owner->onGround)
    {
        if ((gfc_vector2d_magnitude_squared(gfc_vector2d(ballData->owner->velocity.x, ballData->owner->velocity.y))) > MOVE_THRESHOLD * MOVE_THRESHOLD)
            ball_start_dribble(self);
    }

    if (ballData->state == DRIBBLING && ballData->owner != NULL)
    {
        if ((gfc_vector2d_magnitude_squared(gfc_vector2d(ballData->owner->velocity.x, ballData->owner->velocity.y))) > MOVE_THRESHOLD * MOVE_THRESHOLD)
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

    slog("basketball worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    slog("basketball localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);

    float floorZ = collision_get_floor_z() + ballData->radius;
    float floorLocal = ball_floor_local_z(self, ballData);
    float handLocal = ball_hand_local_z(self, ballData);

    //GFC_Vector3D targetPosition = backboard_get_rim_position();

    switch (ballData->state)
    {
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
            if (ballData->owner->onGround)
            {
                self->localPosition = BALL_HOLD_OFFSET;
            }
            else
            {
                ball_jumpshot(self, gfc_vector3d(0,-200,0)); // TODO: this is a hardcoded position. CHANGE LATER
            }
            self->velocity = gfc_vector3d(0,0,0);
            break;
        }
        case DRIBBLING:
        {
            self->localPosition.x = 12;
            self->localPosition.y = 0;
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
    if (self == _basketball)
        _basketball = NULL;
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
    ball->localPosition.y = 0;
    ball->localPosition.z = ball_hand_local_z(ball, ballData);
    ball->velocity.z = -ballData->pushSpeed;

    // reset
    ballData->stillTimer = 0;
    ballData->wantsHold = 0;
}

Entity* ball_get_owner(Entity* self)
{
    if (!self | !self->data) return NULL;
    return ((BallData*)self->data)->owner;
}

Entity* get_game_ball()
{
    return _basketball;
}

void ball_gather(Entity *ball)
{
    BallData *ballData;
    if (!ball || !ball->data) return;
    ballData = (BallData *)ball->data;
    if (!ballData->owner) return;

    ballData->state = HELD;
    ballData->wantsHold = 0;
    ballData->stillTimer = 0;
    ball->velocity = gfc_vector3d(0,0,0);
    ball->localPosition = BALL_SHOOT_OFFSET;
}

void ball_jumpshot(Entity* ball, GFC_Vector3D targetPosition)
{
    BallData* ballData;
    if (!ball || !ball->data) return;
    ballData = (BallData*)ball->data;
    if (!ballData->owner) return;

    GFC_Vector3D pointOfRelease = gfc_vector3d(0,0,0);
    GFC_Vector3D targetHoopPosition = gfc_vector3d(0,-200,0);
    GFC_Vector3D distanceFromHoop = gfc_vector3d(0,0,0);

    ball->localPosition = BALL_SHOOT_OFFSET; // shot position when the player jumps
    gfc_vector3d_add(pointOfRelease, ballData->owner->localPosition, BALL_SHOOT_OFFSET); // point of release is calculated
    gfc_vector3d_sub(distanceFromHoop, targetHoopPosition, pointOfRelease); // gets the distance from the based on the current point of release and hoop pos

    float horizontalDistance = gfc_vector2d_magnitude(gfc_vector2d(distanceFromHoop.x, distanceFromHoop.y));

    float timeInTheAir = SHOT_TIME_BASE + SHOT_TIME_PER_UNIT * horizontalDistance;

    GFC_Vector3D vel;

    vel.x = distanceFromHoop.x / timeInTheAir;
    vel.y = distanceFromHoop.y / timeInTheAir;
    vel.z = (distanceFromHoop.z + 0.5f * GRAVITY * timeInTheAir * timeInTheAir) / timeInTheAir;



}
