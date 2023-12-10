#include "Window.h"

#include "Core/Logger.h"
#include "Core/Global.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

#include "VkRenderer/RenderGlobals.h"
//#include "Renderer/Vulkan.h"

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
    //Abstract away the windows implementation?
    //Leave this here, this is how you do fullscreen properly in GLFW
    //Pass in the monitor to the glfwCreateWindow.
    //GLFWmonitor *const monitor =  glfwGetPrimaryMonitor();
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    //glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    //glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    //glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    //glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    //_windowData.width = mode->width;
    //_windowData.height = mode->height;

    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //Global::getInstance().setWindow(glfwCreateWindow(_windowData.width, _windowData.height, _windowData.title, monitor, nullptr));
    //glfwSetWindowMonitor(Global::getInstance().getWindow(), monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Global::getInstance().setWindow(glfwCreateWindow(_windowData.width, _windowData.height, _windowData.title, nullptr, nullptr));
    Global::getInstance().setWindowExtent(_windowData.width, _windowData.height);
    RenderGlobals::getInstance();


    glfwSetWindowUserPointer(Global::getInstance().getWindow(), &_windowData);

    glfwSetWindowSizeCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, int width, int height)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));
            windowData.width = width;
            windowData.height = height;
            Global::getInstance().setWindowExtent(windowData.width, windowData.height);

            WindowResizeEvent windowResizeEvent(width, height);
            windowData.eventCallback(windowResizeEvent);
        });

    glfwSetWindowCloseCallback(Global::getInstance().getWindow(), [](GLFWwindow* window)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            WindowClosedEvent windowCloseEvent;
            windowData.eventCallback(windowCloseEvent);
        });

    glfwSetKeyCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, int key, int scanCode, int action, int mods)
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
    glfwSetCharCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, uint32_t characters)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            TypedKeyEvent typedKeyPress(characters);
            windowData.eventCallback(typedKeyPress);
        });

    glfwSetMouseButtonCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, int button, int action, int mods)
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

    glfwSetScrollCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, double x, double y)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            MouseScrolledEvent scrollEvent(static_cast<float>(x), static_cast<float>(y));
            windowData.eventCallback(scrollEvent);
        });

    glfwSetCursorPosCallback(Global::getInstance().getWindow(), [](GLFWwindow* window, double x, double y)
        {
            WindowData& windowData = *(WindowData*)(glfwGetWindowUserPointer(window));

            MouseMovedEvent movedEvent(static_cast<float>(x), static_cast<float>(y));
            windowData.eventCallback(movedEvent);
        });
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
}

void Window::waitDevice()
{
    //_vulkan->waitDevice();
}

} //GUST