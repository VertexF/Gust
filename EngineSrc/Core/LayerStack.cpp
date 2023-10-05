#include "LayerStack.h"

#include "Logger.h"
#include "Layer.h"

namespace Gust 
{

LayerStack::LayerStack() : _layerIteratorIndex(0)
{
}

LayerStack::~LayerStack() 
{
    for (Layer *layer : _layers)
    {
        delete layer;
    }
}

void LayerStack::pushLayer(Layer* layer) 
{
    _layers.emplace(_layers.begin() + _layerIteratorIndex, layer);
    _layerIteratorIndex++;
    layer->attach();
}

void LayerStack::popLayer(Layer* layer)
{
    auto it = std::find(_layers.begin(), _layers.begin() + _layerIteratorIndex, layer);
    if (it == _layers.end())
    {
        GUST_CRITICAL("You are trying to pop an empty list");
        return;
    }

    layer->detach();
    _layers.erase(it);
    _layerIteratorIndex--;
}

void LayerStack::attachTopLayer() 
{
    _layers.back()->attach();
}

std::vector<Layer*>::iterator LayerStack::begin() 
{
    return _layers.begin();
}

std::vector<Layer*>::iterator LayerStack::end() 
{
    return _layers.end();
}

std::vector<Layer*>::reverse_iterator LayerStack::rbegin() 
{
    return _layers.rbegin();
}

std::vector<Layer*>::reverse_iterator LayerStack::rend() 
{
    return _layers.rend();
}

Layer* LayerStack::top() 
{
    if (_layers.empty()) 
    {
        GUST_CRITICAL("You are trying to get the top layer of the layer stack on an empty list");
        return nullptr;
    }
    return _layers.back();
}

bool LayerStack::empty() const 
{
    return _layers.empty();
}

}//GUST