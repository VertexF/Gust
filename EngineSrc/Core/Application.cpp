#include "Application.h"

//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Game/Game2D.h"
#include "Window.h"
#include "Global.h"

#include "Core/Instrumentor.h"

namespace Gust 
{

Application::Application(const char* title) : _lastFrameTime(0.f)
{
    GUST_PROFILE_FUNCTION();
    _window = new Window(WindowProperties(title));
    _window->setCallbackFunction(std::bind(&Application::handleEvent, this, std::placeholders::_1));
}

Application::~Application()
{
    delete _window;
}

void Application::handleEvent(Event& ent) 
{
    EventDispatcher eventDispatcher(ent);
    eventDispatcher.dispatch<WindowClosedEvent>(std::bind(&Application::windowClosed, this, std::placeholders::_1));
    eventDispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::windowResize, this, std::placeholders::_1));

    for (auto it = _layers.rbegin(); it != _layers.rend(); it++) 
    {
        if (ent.isHandled) 
        {
            break;
        }

        (*it)->handleEvent(ent);
    }
}

void Application::run() 
{
    GUST_PROFILE_FUNCTION();
    _layers.pushLayer(new game::Game2D());
    while (Global::getInstance().running) 
    {
        //Do the stack.
        float time = static_cast<float>(glfwGetTime());
        TimeStep timestep(time - _lastFrameTime);
        _lastFrameTime = time;

        if (_minimized == false)
        {
            for (Layer* layer : _layers)
            {
                layer->update(timestep);
            }
        }

        _window->update();
    }
}

bool Application::windowClosed(WindowClosedEvent& closed) 
{
    Global::getInstance().running = false;
    return true;
}

bool Application::windowResize(WindowResizeEvent& resized) 
{
    if (resized.getWidth() == 0 || resized.getHeight() == 0) 
    {
        _minimized = true;
        return true;
    }

    _minimized = false;
    return false;

}

} //GUST