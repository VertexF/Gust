#ifndef APPLICATION_HDR
#define APPLICATION_HDR

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "LayerStack.h"

namespace Gust 
{

class Layer;
class Window;

class Application 
{
public:
    Application(const char* title = "Game Engine");
    ~Application();

    void handleEvent(Event& ent);
    void run();

    bool windowClosed(WindowClosedEvent& closed);
    bool windowResize(WindowResizeEvent& resized);

private:
    bool _minimized = false;

    float _lastFrameTime;

    LayerStack _layers;
    Window* _window;
};

}

#endif // !APPLICATION_HDR
