#ifndef MOUSE_EVENTS_HDR
#define MOUSE_EVENTS_HDR

#include "Event.h"

namespace Gust 
{

class MouseMovedEvent : public Event 
{
public:
    MouseMovedEvent(float x, float y);
    virtual ~MouseMovedEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;

    float x;
    float y;
};

class MouseScrolledEvent : public Event
{
public:
    MouseScrolledEvent(float x, float y);
    virtual ~MouseScrolledEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;

    float x;
    float y;
};

class MouseButtonEvent : public Event 
{
public:
    int getMouseButton() const;

protected:
    MouseButtonEvent(int button);
    virtual ~MouseButtonEvent() = default;

    int button;
};


class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    MouseButtonPressedEvent(int button);
    virtual ~MouseButtonPressedEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    MouseButtonReleasedEvent(int button);
    virtual ~MouseButtonReleasedEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

}

#endif // !MOUSE_EVENTS_HDR
