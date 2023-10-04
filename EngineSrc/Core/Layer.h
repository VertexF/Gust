#ifndef LAYER_HDR
#define LAYER_HDR

#include "Events/Event.h"
#include "TimeStep.h"

namespace Gust 
{

class Layer 
{
public:
    Layer(const char* layerName) : _debugName(layerName)
    {
    }
    virtual ~Layer() = default;

    //Used as manual "deconstructors and constructors" when they are popped of the stack.
    virtual void attach() = 0;
    virtual void detach() = 0;
    virtual void update(TimeStep timeStep) = 0;
    virtual void handleEvent(Event &ent) = 0;

    const char* getLayerName() const { return _debugName; }

private:
    const char* _debugName;
};

}

#endif // !LAYER_HDR
