#ifndef VERTEX_HDR
#define VERTEX_HDR

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtx/hash.hpp>

namespace Gust
{

struct Vertex
{

glm::vec3 pos;
glm::vec3 colour;
glm::vec2 texCoord;

static VkVertexInputBindingDescription getBindingDescription();
static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

bool operator==(const Vertex& rhs) const;

};

}//GUST

//Look into your minecraft game when you try to move this.
namespace std
{

template<>
struct std::hash<Gust::Vertex>
{
    std::size_t operator()(Gust::Vertex const& vertex) const
    {
        auto hash1 = std::hash<glm::vec3>()(vertex.pos);
        auto hash2 = std::hash<glm::vec3>()(vertex.colour);
        auto hash3 = std::hash<glm::vec2>()(vertex.texCoord);

        return ((hash1 ^ (hash2 << 1)) >> 1) ^ (hash3 << 1);
    }
};

} //STD


#endif // !VERTEX_HDR