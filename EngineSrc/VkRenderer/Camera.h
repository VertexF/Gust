#ifndef CAMERA_HDR
#define CAMERA_HDR

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include "Core/TimeStep.h"

namespace Gust 
{
class Camera 
{
public:
    Camera();

    void setOrthographicalProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    const glm::mat4& getProjection() const;
    const glm::mat4& getView() const;
    const glm::mat4& getViewProjection() const;
    const glm::mat4& getInverseView() const;
    const glm::vec3 getPosition() const;
private:
    glm::mat4 _projectionMatrix;
    glm::mat4 _viewMatrix;
    glm::mat4 _inverseViewMatrix;

    glm::vec3 _position;
    glm::vec3 _rotation;
};
}

#endif // !CAMERA_HDR
