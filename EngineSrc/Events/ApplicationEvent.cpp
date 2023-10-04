#include "ApplicationEvent.h"

#include "Core/Logger.h"

namespace Gust
{
WindowResizeEvent::WindowResizeEvent(int width, int height) : Event(EVENT_CATEGORY_APPLICATION),
    _width(width), _height(height)
{
}

void WindowResizeEvent::printState() 
{
    GUST_INFO("Windows resize state: {0}, {1}", _width, _height);
}

int WindowResizeEvent::getWidth() const 
{
    return _width;
}

int WindowResizeEvent::getHeight() const 
{
    return _height;
}

EventType WindowResizeEvent::getStaticType() 
{
    return EventType::WINDOW_RESIZE;
}

EventType WindowResizeEvent::getEventType() const 
{
    return getStaticType();
}

const char* WindowResizeEvent::getName() const 
{
    return "Windows resize";
}


WindowClosedEvent::WindowClosedEvent() : Event(EVENT_CATEGORY_APPLICATION)
{
}

EventType WindowClosedEvent::getStaticType()
{
    return EventType::WINDOW_CLOSED;
}

EventType WindowClosedEvent::getEventType() const
{
    return getStaticType();
}

const char* WindowClosedEvent::getName() const
{
    return "Windows close";
}


AppUpdateEvent::AppUpdateEvent() : Event(EVENT_CATEGORY_APPLICATION)
{
}

EventType AppUpdateEvent::getStaticType()
{
    return EventType::APP_UPDATE;
}

EventType AppUpdateEvent::getEventType() const
{
    return getStaticType();
}

const char* AppUpdateEvent::getName() const
{
    return "Application update";
}


AppTickEvent::AppTickEvent() : Event(EVENT_CATEGORY_APPLICATION)
{
}

EventType AppTickEvent::getStaticType()
{
    return EventType::APP_TICK;
}

EventType AppTickEvent::getEventType() const
{
    return getStaticType();
}

const char* AppTickEvent::getName() const
{
    return "Application tick";
}


AppRenderEvent::AppRenderEvent() : Event(EVENT_CATEGORY_APPLICATION)
{
}

EventType AppRenderEvent::getStaticType()
{
    return EventType::APP_RENDER;
}

EventType AppRenderEvent::getEventType() const
{
    return getStaticType();
}

const char* AppRenderEvent::getName() const
{
    return "Application render";
}

}//GUST