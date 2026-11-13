#include "simple_logger.h"

#include "gfc_primitives.h"

#include "entity.h"
#include "player.h"
#include "basketball.h"

#include "collision.h"

void collision_boundary(Entity* entity, GFC_Box bounds)
{
    if (!entity) return;

    // x-axis - left right movement
    if (entity->localPosition.x < bounds.x)
    {
        entity->localPosition.x = bounds.x;
    }
    if (entity->localPosition.x > bounds.x + bounds.w)
    {
        entity->localPosition.x = bounds.x + bounds.w;
    }

    // y-axis - forward backward movement
    if (entity->localPosition.y < bounds.y)
    {
        entity->localPosition.y = bounds.y;
    }
    if (entity->localPosition.y > bounds.y + bounds.h)
    {
        entity->localPosition.y = bounds.y + bounds.h;
    }

    // z-axis - jumping up and down and floor
    float floorZ = bounds.z + bounds.d;
    if (entity->model && entity->model->mesh)
        floorZ -= entity->model->mesh->bounds.z * entity->localScale.z; // this part allows the player's feet to visibly sit on the court

    if (entity->localPosition.z <= floorZ)
    {
        entity->localPosition.z = floorZ;
        if (entity->localPosition.z < 0)
            entity->velocity.z = 0;
        entity->onGround = 1;
    }

    /*if (entity->localPosition.z < bounds.z)
    {
        entity->localPosition.z = bounds.z;
    }
    if (entity->localPosition.z > bounds.z + bounds.d)
    {
        entity->localPosition.z = bounds.z + bounds.d;
    }*/

}

GFC_Box entity_box_collision(Entity* self)
{
    GFC_Box b;

    if (!self || !self->model || !self->model->mesh)
        return gfc_box(0,0,0,0,0,0);

    b = self->model->mesh->bounds;
    return gfc_box(
        self->localPosition.x + b.x * self->localScale.x,
        self->localPosition.y + b.y * self->localScale.y,
        self->localPosition.z + b.z * self->localScale.z,
        b.w * self->localScale.x,
        b.h * self->localScale.y,
        b.d * self->localScale.z
    );
}

int check_box_collision(GFC_Box a, GFC_Box b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x &&
    a.y < b.y + b.h && a.y + a.h > b.y &&
    a.z < b.z + b.d && a.z + a.d > b.z;
}

int check_entity_collision(Entity* a, Entity* b)
{
    if (!a || !b) return 0;
    if (check_box_collision(entity_box_collision(a), entity_box_collision(b)))
    {
        return 1;
    }

    /*GFC_Box playerBox = player_collision_box(player);
    GFC_Box otherBox = basketball_collision_box(other);

    /*float playerX;
    float playerY;
    float playerZ;

    float otherX, otherY, otherZ;

    int result = 0;

    if (player->localPosition.x >= otherBox.x &&
        player->localPosition.x <= otherBox.x + otherBox.x &&
        player->localPosition.y >= otherBox.y &&
        player->localPosition.y <= otherBox.y + otherBox.y &&
        player->localPosition.z >= otherBox.z &&
        player->localPosition.z <= otherBox.z
    )
    {
        result = 1;
    }
    if (result)
    {
        slog ("PLAYER COLLIDING WITH BALL!!");
        return result;
    }
    //slog("otherBox: %f %f %f", otherBox.x, otherBox.y, otherBox.z);*/
}
