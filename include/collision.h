#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "gfc_primitives.h"
#include "entity.h"
#include "level.h"

void resolve_collision_x(Entity* entity, Level* level);

void resolve_collision_y(Entity* entity, Level* level);

void collision_boundary(Entity* entity, GFC_Box bounds);

Bool check_entity_overlap(Entity* a, Entity* b);

int check_entity_collision(Entity* player, Entity* other);

int check_box_collision(GFC_Box a, GFC_Box b) ;

#endif
