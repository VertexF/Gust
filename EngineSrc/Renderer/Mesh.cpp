#include "Mesh.h"

#include "tiny_obj_loader.h"
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

bool Mesh::loadFromObj(const char* filename) 
{
    tinyobj::attrib_t attributes;
    //Shapes contains the info for each seperate object in the file.
    std::vector<tinyobj::shape_t> shapes;
    //Material infomation about each shape.
    std::vector<tinyobj::material_t> materials;

    std::string warning;
    std::string error;

    tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filename, nullptr);
    GUST_WARN("The warnings from loading the obj file are: {0}", warning);
    GUST_ERROR("The errors from loading the obj file are: {0}", error);

    for (size_t i = 0; i < shapes.size(); i++)
    {
        size_t indexOffset = 0;
        for (size_t face = 0; face < shapes[i].mesh.num_face_vertices.size(); face++)
        {
            //NOTE: This is hardcoded to 3.
            int faceVertices = 3;

            for (size_t vertex = 0; vertex < faceVertices; vertex++)
            {
                tinyobj::index_t index = shapes[i].mesh.indices[indexOffset + vertex];
                tinyobj::real_t vx = attributes.vertices[3 * index.vertex_index];
                tinyobj::real_t vy = attributes.vertices[3 * index.vertex_index + 1];
                tinyobj::real_t vz = attributes.vertices[3 * index.vertex_index + 2];

                tinyobj::real_t nx = attributes.normals[3 * index.normal_index];
                tinyobj::real_t ny = attributes.normals[3 * index.normal_index + 1];
                tinyobj::real_t nz = attributes.normals[3 * index.normal_index + 2];

                ModelVertex newVertex;
                newVertex.position.x = vx;
                newVertex.position.y = vy;
                newVertex.position.z = vz;

                newVertex.normal.x = nx;
                newVertex.normal.y = ny;
                newVertex.normal.z = nz;

                //This is just for display purposes 
                newVertex.colour = newVertex.normal;

                vertices.push_back(newVertex);
            }
            indexOffset += faceVertices;
        }
    }

    return true;
}
    
} //GUST