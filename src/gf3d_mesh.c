#include "gfc_types.h"
#include "simple_logger.h"
#include "gf3d_vgraphics.h"


#include "gf3d_buffers.h"
#include "gf3d_device.h"
#include "gf3d_obj_load.h"
#include "gf3d_pipeline.h"
#include <vulkan/vulkan_core.h>

#include "gf3d_mesh.h"
//#include "model.h"

typedef struct
{
    Uint32                              meshCount;
    Mesh                                *meshList;
    VkDevice                            device;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;

}MeshManager;

static MeshManager mesh_manager = {0};

// forward declares
void gf3d_mesh_close(void);
void gf3d_mesh_primitive_free(MeshPrimitive* prim);
void gf3d_mesh_delete(Mesh* mesh);

MeshPrimitive* gf3d_mesh_primitive_new(void);

int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim);

void gf3d_mesh_init(Uint32 mesh_max)
{
    if (mesh_max == 0)
    {
        slog("cannot initialize meshes if there are no meshes");
    }
    mesh_manager.meshCount = mesh_max;
    mesh_manager.meshList = gfc_allocate_array(sizeof(Mesh), mesh_max);
    mesh_manager.device = gf3d_vgraphics_get_default_logical_device();
    // attributes and binding are handled within get_attribute and get_binding

    atexit(gf3d_mesh_close);
}

void gf3d_mesh_close(void)
{
    int i;
    // go through list of meshes and free
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        gf3d_mesh_free(&mesh_manager.meshList[i]);
    }
    free(mesh_manager.meshList);
    memset(&mesh_manager, 0, sizeof(MeshManager));
}

Mesh *gf3d_mesh_new(void)
{
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if(mesh_manager.meshList[i]._refCount == 0)
        {
            mesh_manager.meshList[i].primitives = gfc_list_new();
            if (mesh_manager.meshList[i].primitives == NULL)
            {
                slog("cannot allocate more memory for new mesh");
                return NULL;
            }
            mesh_manager.meshList[i]._refCount = 1;
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}

void gf3d_mesh_free(Mesh *mesh)
{
    if (!mesh) return;
    mesh-> _inuse--;
    if (mesh->_inuse <= 0) gf3d_mesh_delete(mesh);
}

void gf3d_mesh_delete(Mesh* mesh)
{
    if (!mesh)return;

    int i,c;
    MeshPrimitive* prim;

    if (mesh->primitives)
    {
        c = gfc_list_count(mesh->primitives); // list of primitives
        for (i = 0; i < c; i++)
        {
            prim = gfc_list_nth(mesh->primitives, i);
            if (!prim) continue;
            gf3d_mesh_primitive_free(prim);
        }
        gfc_list_delete(mesh->primitives);
    }

    memset(mesh,0,sizeof(Mesh));
}

Mesh* gf3d_mesh_get_by_filename(const char* filename)
{
    if (!filename) return NULL;
    int i;
    for (i = 0; i < mesh_manager.meshCount; i++)
    {
        if(!mesh_manager.meshList[i]._refCount) continue;
        if(gfc_line_cmp(mesh_manager.meshList[i].filename, filename) == 0)
        {
            return &mesh_manager.meshList[i];
        }
    }
    return NULL;
}

Mesh *gf3d_mesh_load_obj(const char *filename)
{
    if (!filename)
    {
        return NULL;
    }

    Mesh* mesh = NULL;
    mesh = gf3d_mesh_get_by_filename(filename);

    if (mesh)
    {
        mesh->_refCount++;
        return mesh;
    }
    mesh = gf3d_mesh_new();
    if (!mesh)
    {
        slog ("FAILED TO CREATE NEW MESH FROM FILE %s", filename);
        return NULL;
    }

    MeshPrimitive* prim;
    prim = gf3d_mesh_primitive_new();
    if (!prim)
    {
        slog("FAIL TO GET MESH PRIM FROM %s", filename);
        return NULL;
    }
    prim->objData = gf3d_obj_load_from_file(filename);
    if (!prim->objData)
    {
        slog("CANNOT OBTAIN OBJ DATA FROM FILE %s", filename);
        gf3d_mesh_delete(mesh);
        return NULL;
    }
    if (!gf3d_mesh_primitive_buffer_create(prim))
    {
        slog("Cannot create primitive buffer from file %s", filename);
        gf3d_mesh_delete(mesh);
        return NULL;
    }

    gfc_list_append(mesh->primitives, prim);
    gfc_line_cpy(mesh->filename, filename);
    return mesh;

}



int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim)
{
    void *data = NULL;


    VkDevice device = mesh_manager.device;
    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if ((!prim) || (!prim->objData))
    {
        slog("buffer_create: NO PRIM OR OBJ DATA");
        return 0;
    }
    slog("obj counts: faces %u, face verts %u", prim->objData->face_count, prim->objData->face_vert_count);

    // face buffers
    bufferSize = sizeof(Face) * prim->objData->face_count;
    prim->faceCount = prim->objData->face_count;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->outFace, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    // vertex buffers
    bufferSize = sizeof(Vertex) * prim->objData->face_vert_count;
    prim->vertexCount = prim->objData->face_vert_count;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, prim->objData->faceVertices, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return 1;
}

void gf3d_mesh_primitive_buffer_free(MeshPrimitive* prim)
{
    if (!prim) return;

    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
    }
    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device,prim->vertexBuffer ,NULL );
    }
    if(prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device,prim->vertexBufferMemory, NULL);
    }

}

MeshPrimitive* gf3d_mesh_primitive_new(void)
{
    MeshPrimitive* prim;
    prim = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    return prim;
}

void gf3d_mesh_primitive_free(MeshPrimitive *prim)
{
    if (!prim) return;
    //prim->_inuse--;
    gf3d_mesh_primitive_buffer_free(prim);
    if (prim->objData)
    {
        gf3d_obj_free(prim->objData);
    }

    free(prim);

}

VkVertexInputAttributeDescription * gf3d_mesh_get_attribute_descriptions(Uint32 *count)
{
    mesh_manager.attributeDescriptions[0].binding = 0;
    mesh_manager.attributeDescriptions[0].location = 0;
    mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    mesh_manager.attributeDescriptions[1].binding = 0;
    mesh_manager.attributeDescriptions[1].location = 1;
    mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    mesh_manager.attributeDescriptions[2].binding = 0;
    mesh_manager.attributeDescriptions[2].location = 2;
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);
    if (count)*count = MESH_ATTRIBUTE_COUNT;
    return mesh_manager.attributeDescriptions;
    if (count)*count = MESH_ATTRIBUTE_COUNT;
    return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription * gf3d_mesh_get_bind_description(void)
{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return &mesh_manager.bindingDescription;
}


void gf3d_mesh_queue_render(Mesh *mesh,Pipeline *pipe,void *uboData,Texture *texture)
{
    int i, c;
    MeshPrimitive* prim;
    // iterate through the list of primitives
    if ((!mesh) || (!mesh->primitives) || (!uboData) || (!texture)) return;
    c = gfc_list_get_count(mesh->primitives);
    for (i = 0; i < c; i++)
    {
        prim = gfc_list_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_pipeline_queue_render(pipe, prim->vertexBuffer, prim->vertexCount, prim->faceBuffer,uboData, texture);
    }
}



