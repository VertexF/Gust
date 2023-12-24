#include "Model.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <glm/gtx/hash.hpp>

#include "Core/Instrumentor.h"

#include "RenderUtils.h"

namespace std 
{

template <>
struct hash<Gust::Model::Vertex> 
{
    size_t operator()(Gust::Model::Vertex const& vertex) const 
    {
        size_t seed = 0;
        Gust::hashCombine(seed, vertex.position, vertex.colour, vertex.normal, vertex.uv);
        return seed;
    }
};

} //STD

namespace Gust 
{

void Model::Builder::loadModel(const std::string& filepath) 
{
    GUST_PROFILE_FUNCTION();
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filepath.c_str());
    GUST_CORE_ASSERT(result == false, "Error : {0} Warning : {1}", warning, error);

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.position =
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };


                vertex.colour =
                {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }

            if (index.normal_index >= 0)
            {
                vertex.normal =
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0)
            {
                vertex.uv =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

Model::Model(const Builder& builder)
{
    GUST_PROFILE_FUNCTION();
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

Model::~Model()
{
    delete _vertexBuffer;

    if (_hasIndexBuffer)
    {
        delete _indexBuffer;
    }
}

Model* Model::createModelFromFile(const std::string& filepath)
{
    GUST_PROFILE_FUNCTION();
    Builder builder = {};
    builder.loadModel(filepath);
    GUST_INFO("Vertices count: {0}", builder.vertices.size());

    return new Model(builder);
}

void Model::bind(VkCommandBuffer commandBuffer) 
{
    GUST_PROFILE_FUNCTION();
    VkBuffer buffers[] = { _vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (_hasIndexBuffer) 
    {
        vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void Model::draw(VkCommandBuffer commandBuffer) 
{
    GUST_PROFILE_FUNCTION();
    if (_hasIndexBuffer) 
    {
        vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, _vertexCount, 1, 0, 0);
    }
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
    GUST_PROFILE_FUNCTION();
    _vertexCount = static_cast<uint32_t>(vertices.size());
    GUST_CORE_ASSERT(_vertexCount < 3, "Vertex count must be at least 3.");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer = { vertexSize, _vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    _vertexBuffer = new Buffer{ vertexSize, _vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    RenderGlobals::getInstance().getDevice()->copyBuffer(stagingBuffer.getBuffer(), _vertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t>& indices)
{
    GUST_PROFILE_FUNCTION();
    _indexCount = static_cast<uint32_t>(indices.size());
    _hasIndexBuffer = _indexCount > 0;
    if (_hasIndexBuffer == false)
    {
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * _indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer = { indexSize, _indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());

    _indexBuffer = new Buffer{ indexSize, _indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    RenderGlobals::getInstance().getDevice()->copyBuffer(stagingBuffer.getBuffer(), _indexBuffer->getBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() 
{
    GUST_PROFILE_FUNCTION();
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;
    bindingDescription[0].stride = sizeof(Vertex);
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
    GUST_PROFILE_FUNCTION();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, colour);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, normal);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
}

bool Model::Vertex::operator==(const Vertex& rhs) const
{
    return (position == rhs.position && colour == rhs.colour && normal == rhs.normal && uv == rhs.uv);
}

}//Gust