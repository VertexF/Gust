#ifndef VERTEX_HDR
#define VERTEX_HDR

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/detail/type_vec3.hpp>

namespace Gust
{

struct Vertex
{

glm::vec3 pos;
glm::vec3 colour;
glm::vec2 texCoord;

static VkVertexInputBindingDescription getBindingDescription();
static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

bool operator==(const Vertex& rhs);

};

}//GUST

#endif // !VERTEX_HDR