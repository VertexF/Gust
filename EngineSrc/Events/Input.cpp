#include "Input.h"

#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "Core/Global.h"

namespace Gust
{

bool Input::isKeyPressed(int keyCode)
{
    auto window = static_cast<GLFWwindow*>(Global::getInstance().getWindow());
    auto state = glfwGetKey(window, keyCode);

    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button)
{
    auto window = static_cast<GLFWwindow*>(Global::getInstance().getWindow());
    auto state = glfwGetMouseButton(window, button);

    return state == GLFW_PRESS;
}

glm::vec2 Input::getMousePosition()
{
    auto window = static_cast<GLFWwindow*>(Global::getInstance().getWindow());
    double xPos, yPos = 0.0;
    glfwGetCursorPos(window, &xPos, &yPos);

    return { static_cast<float>(xPos), static_cast<float>(yPos) };
}

float Input::getMouseX()
{
    return getMousePosition().x;
}

float Input::getMouseY()
{
    return getMousePosition().y;
}

}//Gust