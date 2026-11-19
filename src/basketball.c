#include "simple_logger.h"

#include "player.h"
#include "basketball.h"

#include "collision.h"

#include "backboard.h"

#include "score.h"

#include "powerup.h"
#include "hazard.h"

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
    ballData->shooter = NULL;
    ballData->restitution = 0.85f; // very bouncy. if it equals 1, then that makes it a perfect bounce and no energy is lost
    ballData->handHeight = 1.0f * UNITS_PER_METER;
    ballData->stillTimer = 0;

    self->data = ballData;

    self->type = ENTITY_TYPE_BASKETBALL;
    self->model = model_load("models/basketball.model");
    self->parent = NULL;

    self->localPosition = BALL_RESPAWN_POSITION;
    self->localRotation = gfc_vector3d(0,0,0);
    self->velocity = gfc_vector3d(0,0,0);
    self->localScale = gfc_vector3d(10,10,10);

    if (self->model && self->model->mesh)
        ballData->radius = self->model->mesh->bounds.w * 0.5f * self->localScale.x;
    float fallHeight = ballData->handHeight - ballData->radius;
    float targetHeight = fallHeight * 1.05f;
    float e = ballData->restitution; // bounciness
    ballData->pushSpeed = sqrtf(2.0f * GRAVITY * (targetHeight / (e * e) - fallHeight)); // v = square root of (2g * h)

    self->hidden = 0;

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

    //slog("basketball worldPosition: %f %f %f", self->worldPosition.x, self->worldPosition.y, self->worldPosition.z);
    //slog("basketball localPosition: %f %f %f", self->localPosition.x, self->localPosition.y, self->localPosition.z);

    float floorZ = collision_get_floor_z() + ballData->radius;
    float floorLocal = ball_floor_local_z(self, ballData);
    float handLocal = ball_hand_local_z(self, ballData);

    //GFC_Vector3D targetPosition = backboard_get_rim_position();

    // when ball detaches cooldown so player can't immediately pick it back up
    if (ballData->pickupCooldown > 0)
        ballData->pickupCooldown -= deltaTime;

    switch (ballData->state)
    {
        case LOOSE:
        {
            ball_loose(self, ballData, deltaTime);
            break;
        }
        case HELD:
        {
            if (!ballData->owner)
                break;

            if (ballData->owner->onGround)
            {
                self->localPosition = BALL_HOLD_OFFSET;
            }
            else
            {
                //ball_jumpshot(self, gfc_vector3d(0,-200,0)); // TODO: this is a hardcoded position. CHANGE LATER
                self->localPosition = BALL_SHOOT_OFFSET;
            }
            self->velocity = gfc_vector3d(0,0,0);
            break;
        }
        case DRIBBLING:
        {
            self->localPosition.x = 12;
            self->localPosition.y = 0;
            self->velocity.z -= GRAVITY * deltaTime; // downward bounce
            self->localPosition.z += self->velocity.z * deltaTime;

            if ((self->localPosition.z <= floorLocal) && self->velocity.z < 0)
            {
                GFC_Vector3D ballWorld = ballData->owner->localPosition;
                Uint8 inDeadSpot = hazard_at(ballWorld, HAZARD_DEADSPOT);
                float e;
                if (inDeadSpot)
                    e = 0.3;
                else
                    e = ballData->restitution;

                self->localPosition.z = floorLocal;
                self->velocity.z = -self->velocity.z * e; // bounciness to exert upward from floor to hand

                // check for deadspots
                float riseNeeded = handLocal - floorLocal;
                if (inDeadSpot && self->velocity.z * self->velocity.z < 2.0f * GRAVITY * riseNeeded) // v squared = 2gh
                {
                    GFC_Vector3D carry = ballData->owner->velocity;
                    slog("dead spot! dribble lost!!");
                    ball_detach(self);
                    self->velocity.x = carry.x;
                    self->velocity.y = carry.y;
                    break;
                }
            }

            // if player stops moving then they will hold ball
            if ((self->localPosition.z >= handLocal && self->velocity.z > 0))
            {
                self->localPosition.z = handLocal; // sets it at hand position
                if (ballData->wantsHold)
                {
                    ballData->state = HELD; self->velocity.z = 0; // no ball movement
                }
                else
                    self->velocity.z = -ballData->pushSpeed; // else they will continue dribbling
            }
            break;
        }
        case AIR:
        {
            float previousZ = self->localPosition.z;
            float timeInTheAir;

            ballData->airTime += deltaTime;
            timeInTheAir = ballData->airTime;

            // shot parabola
            self->localPosition.x = ballData->launchPosition.x + ballData->launchVelocity.x * timeInTheAir;
            self->localPosition.y = ballData->launchPosition.y + ballData->launchVelocity.y * timeInTheAir;
            self->localPosition.z = ballData->launchPosition.z + ballData->launchVelocity.z * timeInTheAir - 0.5f * GRAVITY * timeInTheAir * timeInTheAir;

            self->velocity = ballData->launchVelocity;
            self->velocity.z = ballData->launchVelocity.z - GRAVITY * timeInTheAir;

            // backspin visual
            self->localRotation.x += ballData->backspin * deltaTime;

            if (ballData->madeShot && check_entity_collision(self, backboard_get()))
            {
                slog("BASKET SCORED!!!");
                score_add(player_give_points(ballData->shooter));
                ballData->madeShot = 0;
                ballData->state = SCORED;
                ballData->scoredTimer = 0;
                self->velocity.x *= 0.1f;
                self->velocity.y *= 0.1f;
                break;
            }

            // whenever the ball goes past the court and falls to bring it back
            if (self->localPosition.z <= floorZ)
            {
                self->localPosition.z = floorZ;
                ballData->state = LOOSE;
            }
            break;
        }
        case SCORED:
        {
            ballData->scoredTimer += deltaTime;
            if (!self->hidden)
            {
                ball_loose(self, ballData, deltaTime);
                if (ballData->scoredTimer >= SCORED_HIDE_TIME)
                    self->hidden = 1;
            }
            if (ballData->scoredTimer >= SCORED_RESPAWN_TIME)
                ball_respawn(self);
        }
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

    if (ballData->pickupCooldown > 0) return; // can't immediately pick it up
    if (ballData->state != LOOSE) return;
    if (ball->hidden) return;

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
    ballData->pickupCooldown = PICKUP_COOLDOWN;
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
    if (!self || !self->data) return NULL;
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

void ball_jumpshot(Entity* ball, GFC_Vector3D rimPosition)
{
    BallData* ballData;
    if (!ball || !ball->data) return;
    ballData = (BallData*)ball->data;
    if (!ballData->owner) return;

    ballData->state = AIR;
    ball->onGround = 0;
    ballData->backspin = BACKSPIN_RATE;

    // the higher the jump the better the release quality
    float releaseQuality = 1.0f - fabsf(ballData->owner->velocity.z) / JUMP_VELOCITY;
    if (releaseQuality < 0.0f)
        releaseQuality = 0.0f;
    if (releaseQuality > 1.0f)
        releaseQuality = 1.0f;

    GFC_Vector3D pointOfRelease = gfc_vector3d(0,0,0);
    GFC_Vector3D distanceFromHoop = gfc_vector3d(0,0,0);
    GFC_Vector3D targetHoopPosition = rimPosition;

    // spread makes it intentionally clear that the player has missed the shot so the ball will shift to the left or right. handled with crandom
    float spread = MAX_AIM_ERROR * (1.0f - releaseQuality);
    float releaseThreshold;

    if (player_has_powerup(ballData->owner, POWERUP_HOT_HAND))
    {
        releaseThreshold = 0.1; // much bigger window to make the shot
    }
    else
        releaseThreshold = MAKE_THRESHOLD;

    slog("releaseThreshold set to: %.2f", releaseThreshold);

    targetHoopPosition.x += gfc_crandom() * spread;
    targetHoopPosition.y += gfc_crandom() * spread;

    // release point and vector to the target
    gfc_vector3d_add(pointOfRelease, ballData->owner->localPosition, BALL_SHOOT_OFFSET); // point of release is calculated
    gfc_vector3d_sub(distanceFromHoop, targetHoopPosition, pointOfRelease); // gets the distance from the based on the current point of release and hoop pos

    // flight time
    float horizontalDistance = horizontal_distance(pointOfRelease, targetHoopPosition);
    float timeInTheAir = SHOT_TIME_BASE + SHOT_TIME_PER_UNIT * horizontalDistance; // more distance, more time it takes for shot to arrive at rim

    // launch velocity
    GFC_Vector3D vel;
    vel.x = distanceFromHoop.x / timeInTheAir; // speed = distance / time
    vel.y = distanceFromHoop.y / timeInTheAir; // speed = distance / time
    vel.z = (distanceFromHoop.z + 0.5f * GRAVITY * timeInTheAir * timeInTheAir) / timeInTheAir; // 1/2 * g * time squared

    ballData->launchPosition = pointOfRelease;
    ballData->launchVelocity = vel;
    ballData->airTime = 0.0f;
    ballData->rimPosition = rimPosition;
    ballData->shooter = ballData->owner;
    ballData->owner = NULL;

    ball->parent = NULL;
    ball->localPosition = pointOfRelease;
    ball->velocity = vel;

    slog("shot distance: %.1f, time in the air: %.2f, release Quality: %.1f", horizontalDistance, timeInTheAir, releaseQuality);

    // determines if the shot is a make or miss
    ballData->madeShot = (releaseQuality >= releaseThreshold);
}

void ball_loose(Entity* self, BallData* ballData, float deltaTime)
{
    if (!self || !self->data || !ballData) return;

    float floorZ = collision_get_floor_z() + ballData->radius;
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
}

void ball_respawn(Entity* ball)
{
    BallData* ballData;
    if (!ball || !ball->data) return;
    ballData = (BallData*)ball->data;

    ball->parent = NULL;
    ball->localPosition = BALL_RESPAWN_POSITION;
    ball->localRotation = gfc_vector3d(0,0,0);
    ball->velocity = gfc_vector3d(0,0,0);
    ball->onGround = 0;
    ball->hidden = 0;

    ballData->state = LOOSE;
    ballData->owner = NULL;
    ballData->scoredTimer = 0;
    ballData->stillTimer = 0;
}
