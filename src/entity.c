#include "simple_logger.h"

#include "gf3d_vgraphics.h"

#include "entity.h"
#include "basketball.h"
#include "collision.h"

typedef struct
{
    Entity* entityList;
    Uint32  entityMax;
} EntityManager;

void entity_system_close(void);

static EntityManager _entityManager = { 0 }; /* <LOCAL global entity manager>*/

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
    _entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
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
    memset(&_entityManager, 0, sizeof(EntityManager));
}

void entity_clear_all()
{
    int i;
    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (!_entityManager.entityList[i]._refCount) continue; // skip iteration of loop
        entity_free(&_entityManager.entityList[i]);
        _entityManager.entityList[i]._refCount = 0;
    }
}

Entity* entity_new()
{
    int i;
    if (!_entityManager.entityList)
    {
        slog("entity list not initialized!!");
        return NULL;
    }

    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (_entityManager.entityList[i]._refCount) continue; // skip any active entity
        memset(&_entityManager.entityList[i], 0, sizeof(Entity));
        _entityManager.entityList[i]._refCount = 1;
        _entityManager.entityList[i].localScale = gfc_vector3d(1, 1, 1); // default scale
        // set default color

        return &_entityManager.entityList[i];
    }
    slog ("no more available entities");
    return NULL;
}

void entity_free(Entity* self)
{
    if (!self) return;
    model_free(self->model);
    // clean up anything else allocated for an entity here
    if (self->free) self->free (self->data);
}

void entity_destroy(Entity* self)
{
    entity_free(self);
    entity_manager_remove(self);
}

void entity_manager_remove(Entity* self)
{
    int i;
    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (&_entityManager.entityList[i] == self)
        {
            _entityManager.entityList[i]._refCount = 0;
            return;
        }
    }
}

void entity_think(Entity* self)
{
    if (!self) return;
    // any boilerplate think stuff goes here
    if (self->think)self->think(self);
}

void entity_system_think()
{
    int i;
    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (!_entityManager.entityList[i]._refCount) continue; // skip any inactive entities
        entity_think(&_entityManager.entityList[i]);
    }
}

void entity_update(Entity* self)
{
    if (!self) return;
    // boilerplate update stuff goes here
    apply_gravity(self, 0.016f);
    if (self->update) self->update(self);

    if (self->parent)
    {
        gfc_vector3d_add(self->worldPosition, self->parent->worldPosition, self->localPosition);
        gfc_vector3d_add(self->worldRotation, self->parent->worldPosition, self->localRotation);
        self->worldScale = self->localScale;
    }
    else
    {
        self->worldPosition = self->localPosition;
        self->worldRotation = self->localRotation;
        self->worldScale = self->localScale;
    }
}

void entity_system_update()
{
    int i;
    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (!_entityManager.entityList[i]._refCount) continue; // skip inactive entities
        entity_update(&_entityManager.entityList[i]);
    }
}

void entity_draw(Entity* self)
{
    GFC_Matrix4 mat;
    //GFC_Matrix4 *view;

    if (!self) return;

    gfc_matrix4_identity(mat);
    //view = gf3d_vgraphics_get_view_matrix();

    gfc_matrix4_from_vectors(
        mat,
        self->worldPosition,
        self->worldRotation,
        self->worldScale);

    if (self->model)
    {
        model_draw(self->model, mat, self->color);
    }
}

void entity_system_draw()
{
    int i;
    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if(!_entityManager.entityList[i]._refCount) continue; // skip inactive entities
        entity_draw(&_entityManager.entityList[i]);
    }
}

void entity_check_collisions(Entity* player)
{
    int i;

    for (i = 0; i < _entityManager.entityMax; i++)
    {
        if (!_entityManager.entityList[i]._refCount) continue; // if the entity is gone, SKIP
        if (&_entityManager.entityList[i] == player) continue;

        if (check_entity_collision(player, &_entityManager.entityList[i]))
        {
            if (_entityManager.entityList[i].type == ENTITY_TYPE_BASKETBALL)
            {
                slog("player colliding with basketball entity");
                ball_attach(&_entityManager.entityList[i], player);
            }
            if (_entityManager.entityList[i].type == ENTITY_TYPE_BACKBOARD)
            {
                slog("player colliding with backboard entity");
            }
        }
    }
}

void apply_gravity(Entity* self, float deltaTime)
{
    float gravity = 9.81f; // gravity value

    if (!self->onGround)
        self->velocity.z -= gravity * deltaTime; // z-value is downward
}
