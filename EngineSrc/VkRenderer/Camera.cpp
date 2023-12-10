#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"
#include "Core/TimeStep.h"
#include "Events/Input.h"
#include "Events/KeyCodes.h"

namespace Gust
{

//TODO: Do we need to work out the "view" matrix to fit the perspective matrix inside vulkan's volumetric view matrix/vector space.
Camera::Camera() : _projectionMatrix(glm::mat4(1.f)), _viewMatrix(glm::mat4(1.f)), 
_position(glm::vec3(0.f)), _rotation(glm::vec3(0.f))
{
}

void Camera::setOrthographicalProjection(float left, float right, float top, float bottom, float near, float far) 
{
    _projectionMatrix = glm::orthoRH_ZO(left, right, bottom, top, near, far);
}
void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) 
{
    _projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) 
{
    _viewMatrix = glm::lookAtRH(position, direction, up);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) 
{
    setViewDirection(position, target - position, up);
}

void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) 
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
    _viewMatrix = glm::mat4{ 1.f };
    _viewMatrix[0][0] = u.x;
    _viewMatrix[1][0] = u.y;
    _viewMatrix[2][0] = u.z;
    _viewMatrix[0][1] = v.x;
    _viewMatrix[1][1] = v.y;
    _viewMatrix[2][1] = v.z;
    _viewMatrix[0][2] = w.x;
    _viewMatrix[1][2] = w.y;
    _viewMatrix[2][2] = w.z;
    _viewMatrix[3][0] = -glm::dot(u, position);
    _viewMatrix[3][1] = -glm::dot(v, position);
    _viewMatrix[3][2] = -glm::dot(w, position);
}

const glm::mat4& Camera::getProjection() const 
{
    return _projectionMatrix;
}

const glm::mat4& Camera::getView() const
{
    return _viewMatrix;
}

const glm::mat4& Camera::getViewProjection() const 
{
    return getProjection() * getView();
}

}