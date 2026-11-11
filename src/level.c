#include "simple_logger.h"

#include "entity.h"
#include "level.h"

Level* level_load(const char* filename)
{
    Level* level = NULL;
    SJson* json = NULL;
    SJson* level_json = NULL;

    if (!filename)
    {
        slog("no filename provided for level_load");
        return NULL;
    }

    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load file %s", filename);
        return NULL;
    }
    level_json = sj_object_get_value(json, "level");
    if (!level_json)
    {
        slog("%s missing 'level' object", filename);
        sj_free(json);
        return NULL;
    }
}

Level* level_new()
{
    Level* level;

}

void level_free(Level* level)
{
    if (!level) return;

    model_free(level->courtModel);
}

void level_draw(Level* level)
{
    if (!level) return;
}

void level_spawn_entities(Level* level)
{
    if (!level) return;
}
