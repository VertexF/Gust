#include "Application.h"

//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Game/Game2D.h"
#include "Window.h"

namespace Gust 
{

Application::Application(const char* title) : _lastFrameTime(0.f)
{
    _window = new Window(WindowProperties(title));
    _window->setCallbackFunction(std::bind(&Application::handleEvent, this, std::placeholders::_1));
}

Application::~Application()
{

}

void Application::handleEvent(Event& ent) 
{
    EventDispatcher eventDispatcher(ent);
    eventDispatcher.dispatch<WindowClosedEvent>(std::bind(&Application::windowClosed, this, std::placeholders::_1));
    eventDispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::windowResize, this, std::placeholders::_1));

    _layers.top()->handleEvent(ent);
}

void Application::run() 
{
    _layers.pushLayer(new game::Game2D());
    while (_running) 
    {
        //Do the stack.
        float time = static_cast<float>(glfwGetTime());
        TimeStep timestep(time - _lastFrameTime);
        _lastFrameTime = time;

        if (_minimized)
        {
            _layers.top()->update(timestep);
        }

        _window->update();
    }
}

bool Application::windowClosed(WindowClosedEvent& closed) 
{
    _running = false;
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