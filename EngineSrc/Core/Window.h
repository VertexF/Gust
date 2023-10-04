#ifndef WINDOW_HDR
#define WINDOW_HDR

//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>

#include "Events/Event.h"

namespace Gust 
{

struct WindowProperties 
{
    const char* title;
    int width;
    int height;

    WindowProperties(const char* title = "Game Engine", int width = 1280, int height = 720) :
        title(title),
        width(width),
        height(height)
    {
    }
};

class Window 
{
public:
    using EventCallbackFunc = std::function<void(Event&)>;

private:
    struct WindowData
    {
        const char* title;
        int width;
        int height;
        bool vSync;

        EventCallbackFunc eventCallback;
    };

    WindowData _windowData;

public:
    Window(const WindowProperties& props);

    void setCallbackFunction(const EventCallbackFunc& callback);

    int getWidth() const;
    int getHeight() const;
    bool isVSync() const;

    void update();

private:
    void drawFrame();

    GLFWwindow* _window;
};

}

#endif // !WINDOW_HDR