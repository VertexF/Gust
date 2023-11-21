#include "Mesh.h"

#include "tiny_obj_loader.h"
#include "stb_image.h"
#include "RenderGlobals.h"
#include "Core/Logger.h"

namespace Gust
{

VertexInputDescription ModelVertex::getVertexDescription()
{
    VertexInputDescription description;

    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(ModelVertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(ModelVertex, position);

    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.offset = offsetof(ModelVertex, normal);

    VkVertexInputAttributeDescription colourAttribute = {};
    colourAttribute.binding = 0;
    colourAttribute.location = 2;
    colourAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    colourAttribute.offset = offsetof(ModelVertex, colour);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(normalAttribute);
    description.attributes.push_back(colourAttribute);

    return description;
}

void Mesh::loadModel(const char* filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename) == false)
    {
        GUST_WARN("The warnings from loading the obj file are: {0}", warn);
        GUST_ERROR("The errors from loading the obj file are: {0}", err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex = {};

            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.colour = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
                _vertices.push_back(vertex);
            }

            _indices.push_back(uniqueVertices[vertex]);
        }
    }
}


void Mesh::loadModelTexture(const char* filename)
{
    //int texWidth = -1;
    //int texHeight = -1;
    //int texChannel = -1;
    //stbi_uc* pixels = stbi_load("Assets/Textures/room.png", &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
    //VkDeviceSize imageSize = texWidth * texHeight * 4;
    //_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    //GUST_CORE_ASSERT(pixels == nullptr, "Failed to load texture image.");
}

} //GUST