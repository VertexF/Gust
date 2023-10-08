#include "Window.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZEOR_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <stb_image.h>
#include <stb_truetype.h>

#include <tiny_obj_loader.h>

#include "Core/Logger.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Renderer/Vulkan.h"

namespace Gust 
{

Window::Window(const WindowProperties& props) 
{
    _windowData.title = props.title;
    _windowData.width = props.width;
    _windowData.height = props.height;

    int width = 0;
    int height = 0;
    int channels = 0;

    if (glfwInit() == false)
    {
        GUST_CRITICAL("GLFW could not start!\n");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //Abstract away the windows implementation?
    _window = glfwCreateWindow(_windowData.width, _windowData.height, _windowData.title, nullptr, nullptr);

    glfwSetWindowUserPointer(_window, &_windowData);

    glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height) 
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));
            windowData.width = width;
            windowData.height = height;

            WindowResizeEvent windowResizeEvent(width, height);
            windowData.eventCallback(windowResizeEvent);
        });

    glfwSetWindowCloseCallback(_window, [](GLFWwindow* window)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            WindowClosedEvent windowCloseEvent;
            windowData.eventCallback(windowCloseEvent);
        });

    glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scanCode, int action, int mods) 
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));
            switch (action)
            {
            case GLFW_REPEAT:
            {
                //Tested to make sure 1 is the correct callback idea for repeated keys.
                PressedKeyEvent repeatKeyEvent(key, 1);
                windowData.eventCallback(repeatKeyEvent);
                break;
            }
            case GLFW_RELEASE:
            {
                ReleasedKeyEvent releasedKeyEvent(key);
                windowData.eventCallback(releasedKeyEvent);
                break;
            }
            }
        });

    //This is used for typing in characters.
    glfwSetCharCallback(_window, [](GLFWwindow* window, uint32_t characters)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            TypedKeyEvent typedKeyPress(characters);
            windowData.eventCallback(typedKeyPress);
        });

    glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods) 
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            switch (action)
            {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent pressEvent(button);
                windowData.eventCallback(pressEvent);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent releaseEvent(button);
                windowData.eventCallback(releaseEvent);
                break;
            }
            };
        });

    glfwSetScrollCallback(_window, [](GLFWwindow* window, double x, double y)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            MouseScrolledEvent scrollEvent(static_cast<float>(x), static_cast<float>(y));
            windowData.eventCallback(scrollEvent);
        });

    glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double x, double y)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            MouseMovedEvent movedEvent(static_cast<float>(x), static_cast<float>(y));
            windowData.eventCallback(movedEvent);
        });

    _vulkan = new Vulkan(_windowData.title, _window);
}

void Window::setCallbackFunction(const EventCallbackFunc& callback)
{
    _windowData.eventCallback = callback;
}

int Window::getWidth() const
{
    return _windowData.width;
}

int Window::getHeight() const
{
    return _windowData.height;
}

bool Window::isVSync() const
{
    return _windowData.vSync;
}

void Window::update()
{
    glfwPollEvents();

    drawFrame();
}

void Window::drawFrame()
{
    //TODO: add the vulkan stuff.
}

} //GUST