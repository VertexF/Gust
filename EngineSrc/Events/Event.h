#ifndef EVENT_HDR
#define EVENT_HDR

//Stub class: TODO implement events.
namespace Gust
{

enum class EventType
{
    NONE = 0,
    WINDOW_CLOSED, WINDOW_RESIZE, WINDOW_FOCUS, WINDOW_FOCUS_LOST, WINDOW_MOVED,
    APP_TICK, APP_UPDATE, APP_RENDER,
    KEY_PRESSED, KEY_RELEASED, KEY_TYPED,
    MOUSE_BUTTON_PRESSED, MOUSE_BUTTON_RELEASED, MOUSE_MOVED, MOUSE_SCROLLED
};

enum EventCategory 
{
    NONE = 0,
    EVENT_CATEGORY_APPLICATION = (1 << 0),
    EVENT_CATEGORY_INPUT = (1 << 1),
    EVENT_CATEGORY_KEYBOARD = (1 << 2),
    EVENT_CATEGORY_MOUSE = (1 << 3),
    EVENT_CATEGORY_MOUSEBUTTON = (1 << 4),
};

class Event
{
public:
    Event(int cat);
    virtual ~Event() = default;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    int getCategoryFlag() const;

    bool isInCategory(EventCategory cat);

    bool isHandled;
private:
    int _category;
};


class EventDispatcher 
{
public:
    EventDispatcher(Event &events);

    template<typename T, typename F>
    bool dispatch(const F& func)
    {
        if (_events.getEventType() == T::getStaticType())
        {
            _events.isHandled |= func(static_cast<T&>(_events));
            return true;
        }

        return false;
    }

private:
    Event& _events;
};

}

#endif // !EVENT_HDR
