#include "Vertex.h"

#include <functional>
#include <glm/gtx/hash.hpp>

namespace Gust
{

VkVertexInputBindingDescription Vertex::getBindingDescription() 
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescription() 
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescription = {};
    attributeDescription[0].binding = 0;
    attributeDescription[0].location = 0;
    attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription[0].offset = offsetof(Vertex, pos);

    attributeDescription[1].binding = 0;
    attributeDescription[1].location = 1;
    attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription[1].offset = offsetof(Vertex, colour);

    attributeDescription[2].binding = 0;
    attributeDescription[2].location = 2;
    attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescription[2].offset = offsetof(Vertex, texCoord);

    return attributeDescription;
}

bool Vertex::operator==(const Vertex& rhs) 
{
    return pos == rhs.pos && colour == rhs.colour && texCoord == rhs.texCoord;
}

} //GUST

//Look into your minecraft game when you try to move this.
namespace std
{

template<>
struct std::hash<Gust::Vertex>
{
    std::size_t operator()(Gust::Vertex const& vertex)
    {
        auto hash1 = std::hash<glm::vec3>()(vertex.pos);
        auto hash2 = std::hash<glm::vec3>()(vertex.colour);
        auto hash3 = std::hash<glm::vec2>()(vertex.texCoord);

        return ((hash1 ^ (hash2 << 1)) >> 1) ^ (hash3 << 1);
    }
};

} //STD