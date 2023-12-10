#include "RenderGlobals.h"

namespace Gust 
{

RenderGlobals& RenderGlobals::getInstance()
{
    static RenderGlobals instance;
    return instance;
}

Device* RenderGlobals::getDevice() const
{
    return _device;
}

RenderGlobals::RenderGlobals()
{
    _device = new Device();
}

RenderGlobals::~RenderGlobals()
{
    //TODO: Fix deletes too early.
    //delete _device;
}

}//Gust