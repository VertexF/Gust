#ifndef APPLICATION_EVENT_HDR
#define APPLICATION_EVENT_HDR

#include "Event.h"

namespace Gust 
{

class WindowResizeEvent : public Event 
{
public:
    WindowResizeEvent(int width, int height);
    virtual ~WindowResizeEvent() = default;

    void printState();

    int getWidth() const;
    int getHeight() const;

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
private:
    int _width;
    int _height;
};

class WindowClosedEvent : public Event
{
public:
    WindowClosedEvent();
    virtual ~WindowClosedEvent() = default;

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

class AppUpdateEvent : public Event
{
public:
    AppUpdateEvent();
    virtual ~AppUpdateEvent() = default;

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

class AppTickEvent : public Event
{
public:
    AppTickEvent();
    virtual ~AppTickEvent() = default;

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

class AppRenderEvent : public Event
{
public:
    AppRenderEvent();
    virtual ~AppRenderEvent() = default;

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

}

#endif // !APPLICATION_EVENT_HDR
