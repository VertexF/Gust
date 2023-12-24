#ifndef RIGID_BODY_HDR
#define RIGID_BODY_HDR

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Gust 
{
//TODO: Actually add physics for things.
class RigidBody 
{
public:
    RigidBody() = default;
    ~RigidBody() = default;

    glm::vec3 translation{};
    glm::vec3 scale{ 1.f, 1.f, 1.f };
    glm::vec3 rotation{};

    glm::mat4 mat4();
    glm::mat4 normalMatrix();
};

}//Gust

#endif // !RIGID_BODY_HDR
