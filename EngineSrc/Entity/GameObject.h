#ifndef GAME_OBJECT_HDR
#define GAME_OBJECT_HDR

#include <cstdint>

#include "VkRenderer/Model.h"
#include "Physics/RigidBody.h"

namespace Gust 
{
class Model;

struct PointLightComponent 
{
    float lightIntensity = 1.f;
};

enum State : uint64_t
{
    ACTIVE = 1 << 0
};

class GameObject 
{
public:
    static GameObject createGameObject();

    int getID();

    static GameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 colour = glm::vec3(1.f));

    glm::vec3 colour;
    //Make this private at somet point, this should be only updated via the physic code path.
    RigidBody transform;

    //These are optional for different entity set ups.
    Model* model;
    PointLightComponent* pointLight;
    uint64_t currentState;

    GameObject(const GameObject& rhs) = delete;
    GameObject& operator=(const GameObject& rhs) = delete;
    GameObject(GameObject&& rhs) = default;
    GameObject& operator=(GameObject&& rhs) = default;
private:
    GameObject(uint32_t objectID);

    uint32_t _objectID;
};

}//Gust

#endif // !GAME_OBJECT_HDR
