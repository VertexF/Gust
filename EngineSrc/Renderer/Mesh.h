#ifndef MESH_HDR
#define MESH_HDR

#include <vector>

#include "vulkan/vulkan.h"
#include "glm/vec3.hpp"
#include "VulkanTypes.h"
#include "Vertex.h"

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
    //TODO: Remove these 2 after the refactoring is done.
    std::vector<ModelVertex> vertices;
    AllocatedBuffer vertexBuffer;

    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    void loadModel(const char* filename);
    void loadModelTexture(const char* filename);
};

}

#endif // !MESH_HDR
