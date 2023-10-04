#include "LayerStack.h"

#include "Logger.h"
#include "Layer.h"

namespace Gust 
{

LayerStack::~LayerStack() 
{
    while (_layers.empty())
    {
        //Make sure this doesn't crash things.
        delete _layers.top();
        _layers.pop();
    }
}

void LayerStack::pushLayer(Layer* layer) 
{
    layer->attach();
    _layers.push(layer);
}

void LayerStack::popLayer() 
{
    if (_layers.empty())
    {
        GUST_CRITICAL("You are trying to pop an empty list");
        return;
    }

    _layers.top()->detach();
    delete _layers.top();
    _layers.pop();
}

Layer* LayerStack::top() 
{
    if (_layers.empty()) 
    {
        GUST_CRITICAL("You are trying to get the top layer of the layer stack on an empty list");
        return nullptr;
    }
    return _layers.top();
}

bool LayerStack::empty() const 
{
    return _layers.empty();
}

}//GUST