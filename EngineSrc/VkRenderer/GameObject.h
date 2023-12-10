#ifndef GAME_OBJECT_HDR
#define GAME_OBJECT_HDR

#include <cstdint>

#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

namespace Gust 
{
struct TransformComponent 
{
glm::vec3 translation{};
glm::vec3 scale{ 1.f, 1.f, 1.f };
glm::vec3 rotation{};

glm::mat4 mat4();
glm::mat4 normalMatrix();
};

class GameObject 
{
public:
    uint32_t objectID;

    static GameObject createGameObject() 
    {
        static uint32_t currentID = 0;
        return GameObject(currentID++);
    }

    int getId() { return objectID; }

    Model* model;
    glm::vec3 colour;
    TransformComponent transform;

    GameObject(const GameObject& rhs) = delete;
    GameObject& operator=(const GameObject& rhs) = delete;
    GameObject(GameObject&& rhs) = default;
    GameObject& operator=(GameObject&& rhs) = default;
private:
    GameObject(uint32_t objectID) : objectID(objectID), model(nullptr)
    {
    }
};

}//Gust

#endif // !GAME_OBJECT_HDR
