#include "MouseEvent.h"

#include "Core/Logger.h"

namespace Gust 
{

MouseMovedEvent::MouseMovedEvent(float x, float y) : Event(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT),
    x(x), y(y)
{
}

void MouseMovedEvent::printState()
{
    GUST_INFO("Mouse position x:{0} , y:{1}", x, y);
}

EventType MouseMovedEvent::getStaticType()
{
    return EventType::MOUSE_MOVED;
}

EventType MouseMovedEvent::getEventType() const
{
    return getStaticType();
}

const char* MouseMovedEvent::getName() const
{
    return "Mouse moved";
}


MouseScrolledEvent::MouseScrolledEvent(float x, float y) : Event(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT),
x(x), y(y)
{
}

void MouseScrolledEvent::printState()
{
    GUST_INFO("Mouse position scrolled x:{0} , y:{1}", x, y);
}

EventType MouseScrolledEvent::getStaticType()
{
    return EventType::MOUSE_SCROLLED;
}

EventType MouseScrolledEvent::getEventType() const
{
    return getStaticType();
}

const char* MouseScrolledEvent::getName() const
{
    return "Mouse scrolled";
}


MouseButtonEvent::MouseButtonEvent(int button) : Event(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSEBUTTON),
    button(button)
{
}

int MouseButtonEvent::getMouseButton() const 
{
    return button;
}


MouseButtonPressedEvent::MouseButtonPressedEvent(int button) : MouseButtonEvent(button)
{
}

void MouseButtonPressedEvent::printState()
{
    GUST_INFO("Mouse button pressed state {0}", button);
}

EventType MouseButtonPressedEvent::getStaticType()
{
    return EventType::MOUSE_BUTTON_PRESSED;
}

EventType MouseButtonPressedEvent::getEventType() const
{
    return getStaticType();
}

const char* MouseButtonPressedEvent::getName() const
{
    return "Mouse button pressed";
}


MouseButtonReleasedEvent::MouseButtonReleasedEvent(int button) : MouseButtonEvent(button)
{
}

void MouseButtonReleasedEvent::printState()
{
    GUST_INFO("Mouse button pressed state {0}", button);
}

EventType MouseButtonReleasedEvent::getStaticType()
{
    return EventType::MOUSE_BUTTON_RELEASED;
}

EventType MouseButtonReleasedEvent::getEventType() const
{
    return getStaticType();
}

const char* MouseButtonReleasedEvent::getName() const
{
    return "Mouse button released";
}

}//GUST