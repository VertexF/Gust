#ifndef MODEL_HDR
#define MODEL_HDR

#include <vector>

#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include "RenderGlobals.h"
#include "Buffer.h"

namespace Gust 
{
class Model 
{
public:

    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 colour;
        glm::vec3 normal;
        glm::vec2 uv;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& rhs) const;
    };

    //This stages the memory before it's copied over into vertex and index buffer.
    struct Builder 
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModel(const std::string& filepath);
    };

    Model(const Builder& builder);
    ~Model();

    Model(const Model& rhs) = delete;
    Model& operator=(const Model& rhs) = delete;

    static Model* createModelFromFile(const std::string& filepath);

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);
private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t>& indices);

    Buffer* _vertexBuffer;
    uint32_t _vertexCount;

    bool _hasIndexBuffer = false;
    Buffer* _indexBuffer = nullptr;
    uint32_t _indexCount;
};
}//Gust

#endif // !MODEL_HDR
