#ifndef LAYER_STACK_HDR
#define LAYER_STACK_HDR

#include <vector>

namespace Gust
{
class Layer;

class LayerStack 
{
public:
    LayerStack();
    ~LayerStack(); 

    void pushLayer(Layer *layer);
    void popLayer(Layer* layer);

    void attachTopLayer();

    std::vector<Layer*>::iterator begin();
    std::vector<Layer*>::iterator end();
    std::vector<Layer*>::reverse_iterator rbegin();
    std::vector<Layer*>::reverse_iterator rend();

    Layer* top();
    bool empty() const;
private:
    std::vector<Layer*> _layers;
    int _layerIteratorIndex;
};

}

#endif // !LAYER_STACK_HDR
