#include "KeyEvent.h"

#include "Core/Logger.h"

namespace Gust 
{

KeyEvent::KeyEvent(int keyCode) : Event(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT), 
    keycode(keyCode)
{
}

int KeyEvent::getKeyCode() const
{
    return keycode;
}


PressedKeyEvent::PressedKeyEvent(int keycode, int repeat) : KeyEvent(keycode), _keyRepeated(repeat)
{
}

int PressedKeyEvent::getTimesRepeated() const 
{
    return _keyRepeated;
}

void PressedKeyEvent::printState() 
{
    GUST_INFO("Key pressed {0}", keycode);
}

EventType PressedKeyEvent::getStaticType() 
{
    return EventType::KEY_PRESSED;
}

EventType PressedKeyEvent::getEventType() const 
{
    return getStaticType();
}

const char* PressedKeyEvent::getName() const 
{
    return "Key pressed";
}


ReleasedKeyEvent::ReleasedKeyEvent(int keycode) : KeyEvent(keycode)
{
}

void ReleasedKeyEvent::printState()
{
    GUST_INFO("Key released {0}", keycode);
}

EventType ReleasedKeyEvent::getStaticType()
{
    return EventType::KEY_RELEASED;
}

EventType ReleasedKeyEvent::getEventType() const
{
    return getStaticType();
}

const char* ReleasedKeyEvent::getName() const
{
    return "Key released";
}


TypedKeyEvent::TypedKeyEvent(int keycode) : KeyEvent(keycode)
{
}

void TypedKeyEvent::printState()
{
    GUST_INFO("Key pressed {0}", keycode);
}

EventType TypedKeyEvent::getStaticType()
{
    return EventType::KEY_TYPED;
}

EventType TypedKeyEvent::getEventType() const
{
    return getStaticType();
}

const char* TypedKeyEvent::getName() const
{
    return "Key typed";
}

} //GUST