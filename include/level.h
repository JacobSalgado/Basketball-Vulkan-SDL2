#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "model.h"
#include "gfc_primitives.h"

typedef struct
{
    Model*      courtModel;
    GFC_Vector3D        courtPosition;
    GFC_Vector3D        courtRotation;
    GFC_Vector3D        courtScale;
    GFC_Box     bounds;         // level bounds so player can't escape
    SJson*      entitiesJson;
    SJson*      _json;
} Level;

/**
 * @brief load a level from config file
 * @param filename the name of level file to load
 */
Level* level_load(const char* filename);

Level* level_new();

void level_free(Level* level);

void level_draw(Level* level);

void level_spawn_entities(Level* level);

#endif
