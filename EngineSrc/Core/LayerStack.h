#ifndef LAYER_STACK_HDR
#define LAYER_STACK_HDR

#include <stack>

namespace Gust
{
class Layer;

class LayerStack 
{
public:
    LayerStack() = default;
    ~LayerStack(); 

    void pushLayer(Layer *layer);
    void popLayer();

    Layer* top();
    bool empty() const;

private:
    std::stack<Layer*> _layers;
};

}

#endif // !LAYER_STACK_HDR
