#ifndef __MODEL_H__
#define __MODEL_H__

#include "gf3d_mesh.h"

typedef struct
{
    Uint8               _inuse;
    Uint32              _refCount;
    GFC_TextLine        filename;
    Texture             *texture;
    Mesh                *mesh; // mesh contains the buffers
}Model;

typedef struct
{
    GFC_Matrix4     model;
    GFC_Matrix4     view;
    GFC_Matrix4     proj;
    GFC_Vector4D    color;
    GFC_Vector4D    camera;
}ModelUBO;

void model_init_system(Uint32 modelCount);

void model_close_system();

void model_free(Model* model);

void model_delete(Model* model);

Model* model_new(void);

/**
 * @brief render a model through a given pipeline
 */
void model_queue_render(Model *model,void *uboData, GFC_Matrix4 mat, GFC_Color colorMod);

ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod);

Model *model_load(const char *filename);

Model* model_get_by_filename(const char* filename);

Pipeline* model_get_pipeline(void);

#endif
