#include "obj.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include "vulkan/buffer.hpp"
#include "vulkan/mesh.hpp"
#include "vulkan/types.hpp"

CVulkanMesh CObjImporter::Load()
{
    return CVulkanMesh();
}
