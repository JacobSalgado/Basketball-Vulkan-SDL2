#include "gf3d_mesh.h"
#include "simple_logger.h"
#include "simple_json.h"

#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"

#include "model.h"

typedef struct
{
    Model* modelList;
    Uint32 modelCount;
    Uint32 chainLength;
    Pipeline *pipe;
    VkDevice device;
    Texture *defaultTexture;
}ModelManager;

static ModelManager model_manager = {0};

void model_close(void);

void model_init_system(Uint32 modelCount)
{
    if (modelCount == 0)
    {
        slog("cannot initialize model system since there are no models");
        return;
    }
    model_manager.chainLength = gf3d_swapchain_get_chain_length();
    model_manager.device = gf3d_vgraphics_get_default_logical_device();
    model_manager.modelList = gfc_allocate_array(sizeof(Model), modelCount);

    model_manager.modelCount = modelCount;

    gf3d_mesh_init(modelCount);

    // pipeline call

    model_manager.pipe = gf3d_pipeline_create_from_config(
        model_manager.device,
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        modelCount,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        MESH_ATTRIBUTE_COUNT,
        sizeof(ModelUBO),
        VK_INDEX_TYPE_UINT32
    );
    if (!model_manager.pipe)
    {
        slog("FAILED TO CREATE PIPELINE FOR MODELS");
        slog_sync();
        model_close();
        return;
    }

    // TODO: insert skybox pipe

    model_manager.defaultTexture = gf3d_texture_load("images/default.png");
    atexit(model_close);
}

void model_free(Model* model)
{
    if (!model)return;
}

void model_delete(Model* model)
{
    if (!model) return;
    gf3d_texture_free(model->texture);
    gf3d_mesh_free(model->mesh);
    memset(model,0,sizeof(Model));
}


Model* model_new(void)
{
    int i;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if(model_manager.modelList[i]._refCount == 0)
        {
            model_delete(&model_manager.modelList[i]);
            model_manager.modelList[i]._refCount = 1;
            return &model_manager.modelList[i];
        }
    }
    return NULL;
}

void model_close(void)
{
    int i;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        model_delete(&model_manager.modelList[i]);
    }
    free(model_manager.modelList);
    memset(&model_manager, 0, sizeof(ModelManager));
}

void model_queue_render(Model *model, GFC_Matrix4 mat, GFC_Color colorMod)
{
    ModelUBO ubo;
    if (!model) return;
    ubo = model_get_ubo(mat, colorMod);
    gf3d_mesh_queue_render(model->mesh, model_manager.pipe,&ubo, model->texture);
}

ModelUBO model_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod)
{
    ModelUBO ubo = {0};
    GFC_Matrix4 *view;

    gfc_matrix4_copy(ubo.model, modelMat);
    view = gf3d_vgraphics_get_view_matrix();
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    ubo.color = gfc_color_to_vector4f(colorMod);

    return ubo;
}

Model *model_load(const char *filename)
{
    Mesh* mesh;
    Model* model;
    Texture* texture;

    const char* str = NULL;
    const char* str2 = NULL;

    SJson *json;
    SJson *data;

    if (!filename) return NULL;
    model = model_get_by_filename(filename);
    if (model)
    {
        //model->_inuse++;
        model->_refCount++;
        slog("model loaded: %s", filename);
        return model;
    }
    json = sj_load(filename);
    if (!json)
    {
        slog("CANNOT LOAD MODEL FILE %s", filename);
        return NULL;
    }
    data = sj_object_get_value(json, "model");
    if (!data)
    {
            slog("CANNOT GET MODEL DATA FROM FILE %s", filename);
            sj_free(json);
            return NULL;

    }
    str = sj_object_get_string(data, "obj");
    if (!str)
    {
        slog ("CANNOT GET OBJ DATA FROM FILE %s", filename);
        sj_free(json);
        return NULL;
    }
    mesh = gf3d_mesh_load_obj(str);
    if (!mesh)
    {
        slog("CANNOT LOAD MESH FROM OBJ DATA %s", filename);
        sj_free(json);
        return NULL;
    }

    str2 = sj_object_get_string(data, "texture");
    if (str2)
    {
        texture = gf3d_texture_load(str2);
        if (!texture)
        {
            slog("FAILED TO LOAD TEXTURE FROM FILE %s", filename);
            texture = model_manager.defaultTexture;
        }
    }
    model = model_new();
    if (!model)
    {
        slog ("Can't get model for model %s", filename);
        return NULL;
    }
    /*model->texture = gf3d_texture_load(filename);
    if (!model->texture)
    {
        slog("model_load: failed to load texture for model. will use default texture");
        model_free(model);
        return NULL;
    }*/
    model->mesh = mesh;
    model->texture = texture;
    gfc_line_cpy(model->filename, filename);

    slog("model loaded: %s", filename);
    return model;
}

Model* model_get_by_filename(const char* filename)
{
    if (!filename) return NULL;
    int i;
    for (i = 0; i < model_manager.modelCount; i++)
    {
        if(!model_manager.modelList[i]._refCount) continue;
        if(gfc_line_cmp(model_manager.modelList[i].filename, filename) == 0)
        {
            return &model_manager.modelList[i];
        }
    }
    return NULL;
}

Pipeline *gf3d_model_get_pipeline(void)
{
    return model_manager.pipe;
}
