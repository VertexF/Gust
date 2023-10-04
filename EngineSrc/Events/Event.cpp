#include "Event.h"

namespace Gust
{

Event::Event(int cat) : _category(cat), isHandled(false)
{
}

int Event::getCategoryFlag() const
{
    return _category;
}

bool Event::isInCategory(EventCategory cat) 
{
    return getCategoryFlag() & cat;
}

EventDispatcher::EventDispatcher(Event& events) : _events(events)
{
}

} //GUST