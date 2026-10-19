#include "simple_logger"

#include "entity.h"

typedef struct
{
    Entity* entityList;
    Uint32  entityMax;
} EntityManager;

static EntityManager _entityManager = {0}

void entity_system_init(Uint32 max)
{
    if (_entityManager.entityList)
    {
        slog("cannot have two instances of an entity manager since on is already active");
        return;
    }

    if (!max)
    {
        slog("cannot allocate 0 entities");
        return;
    }
    _entityManager.entityList = gfc_allocate_array(sizeOf(Entity), max);
    if (!_entityManager.entityList)
    {
        slog("failed to allocate global entity list");
        return;
    }
    _entityManager.entityMax = max;
    slog("entity system initialized!!");
    atexit(entity_system_close);
}

void entity_system_close()
{
    entity_clear_all();
    if(_entityManager.entityList)free(_entityManager.entityList);
    memset(&_entityManager, 0, sizeOf(EntityManager));
}

void entity_clear_all()
{

}

Entity* entity_new()
{

}

void entity_free(Entity* self)
{

}

void entity_system_think()
{

}

void entity_system_update()
{

}
