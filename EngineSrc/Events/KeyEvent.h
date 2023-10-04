#ifndef KEY_EVENT_HDR
#define KEY_EVENT_HDR

#include "Event.h"

namespace Gust 
{

class KeyEvent : public Event 
{
public:
    int getKeyCode() const;

protected:
    KeyEvent(int keyCode);
    virtual ~KeyEvent() = default;

    int keycode;
};

class PressedKeyEvent : public KeyEvent 
{
public:
    PressedKeyEvent(int keycode, int repeat);
    virtual ~PressedKeyEvent() = default;

    int getTimesRepeated() const;
    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
private:
    int _keyRepeated;
};

class ReleasedKeyEvent : public KeyEvent
{
public:
    ReleasedKeyEvent(int keycode);
    virtual ~ReleasedKeyEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

//Exactly as the PressedKeyEvent class but with no repeats.
class TypedKeyEvent : public KeyEvent
{
public:
    TypedKeyEvent(int keycode);
    virtual ~TypedKeyEvent() = default;

    void printState();

    static EventType getStaticType();
    virtual EventType getEventType() const override;
    virtual const char* getName() const override;
};

} //GUST

#endif // !KEY_EVENT_HDR
