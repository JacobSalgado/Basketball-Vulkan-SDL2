#include "simple_logger.h"
#include "game_physics.h"

float horizontal_distance(GFC_Vector3D a, GFC_Vector3D b)
{
    return gfc_vector2d_magnitude(gfc_vector2d(b.x - a.x, b.y - a.y));
}
