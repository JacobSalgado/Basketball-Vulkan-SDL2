#include "simple_logger.h"

#include "collision.h"

void collision_boundary(Entity* entity, GFC_Box bounds)
{
    if (!entity) return;

    if (entity->localPosition.x < bounds.x)
    {
        entity->localPosition.x = bounds.x;
    }
    if (entity->localPosition.x > bounds.x + bounds.w)
    {
        entity->localPosition.x = bounds.x + bounds.w;
    }
    if (entity->localPosition.y < bounds.y)
    {
        entity->localPosition.y = bounds.y;
    }
    if (entity->localPosition.y > bounds.y + bounds.h)
    {
        entity->localPosition.y = bounds.y + bounds.h;
    }

}
