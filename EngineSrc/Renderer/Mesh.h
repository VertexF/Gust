#ifndef MESH_HDR
#define MESH_HDR

#include <vector>

#include "vulkan/vulkan.h"
#include "glm/vec3.hpp"
#include "VulkanTypes.h"

namespace Gust 
{

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

//TODO: Change this back to Vertex when you have removed the other render/merged it in.
struct ModelVertex 
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 colour;

    static VertexInputDescription getVertexDescription();
};

struct Mesh 
{
    std::vector<ModelVertex> vertices;
    AllocatedBuffer vertexBuffer;

    bool loadFromObj(const char *filename);
};

}

#endif // !MESH_HDR
