#include "CameraController.h"

#include <limits>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/Instrumentor.h"

#include "Events/Input.h"
#include "Events/KeyCodes.h"

namespace Gust 
{
void CameraController::moveInPlaneXZ(TimeStep timestep, GameObject& gameObjects)
{
    GUST_PROFILE_FUNCTION();
    glm::vec3 rotate{ 0.f };

    if (Input::isKeyPressed(TEMP_KEY_RIGHT)) 
    {
        rotate.y += 1.0f;
    }
    else if (Input::isKeyPressed(TEMP_KEY_LEFT)) 
    {
        rotate.y -= 1.0f;
    }

    if (Input::isKeyPressed(TEMP_KEY_UP))
    {
        rotate.x += 1.0f;
    }
    else if (Input::isKeyPressed(TEMP_KEY_DOWN))
    {
        rotate.x -= 1.0f;
    }

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
    {
        gameObjects.transform.rotation += _lookSpeed * timestep * glm::normalize(rotate);
    }

    gameObjects.transform.rotation.x = glm::clamp(gameObjects.transform.rotation.x, -1.5f, 1.5f);
    gameObjects.transform.rotation.y = glm::mod(gameObjects.transform.rotation.y, glm::two_pi<float>());

    float yaw = gameObjects.transform.rotation.y;
    const glm::vec3 forwardDir{ glm::sin(yaw), 0.f, glm::cos(yaw) };
    const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
    const glm::vec3 upDir{ 0.f, -1.f, 0.f };

    glm::vec3 moveDir{ 0.f };
    if (Input::isKeyPressed(TEMP_KEY_W)) 
    {
        moveDir -= forwardDir;
    }
    if (Input::isKeyPressed(TEMP_KEY_S)) 
    {
        moveDir += forwardDir;
    }
    if (Input::isKeyPressed(TEMP_KEY_D))
    {
        moveDir += rightDir;
    }
    if (Input::isKeyPressed(TEMP_KEY_A))
    {
        moveDir -= rightDir;
    }
    if (Input::isKeyPressed(TEMP_KEY_E)) 
    {
        moveDir += upDir;
    }
    if (Input::isKeyPressed(TEMP_KEY_Q)) 
    {
        moveDir -= upDir;
    }

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    {
        gameObjects.transform.translation += _moveSpeed * timestep * glm::normalize(moveDir);
    }
}
}//Gust