#ifndef RENDER_GLOBALS_HDR
#define RENDER_GLOBALS_HDR

#include "Device.h"

namespace Gust 
{
class RenderGlobals
{
public:
    static RenderGlobals& getInstance();

    Device* getDevice() const;
private:
    RenderGlobals();
    ~RenderGlobals();

    Device *_device;

public:
    RenderGlobals(RenderGlobals const& rhs) = delete;
    RenderGlobals(RenderGlobals&& rhs) = delete;
    void operator=(RenderGlobals const& rhs) = delete;
    RenderGlobals operator=(RenderGlobals&& rhs) = delete;
};
}

#endif // !RENDER_GLOBALS_HDR
