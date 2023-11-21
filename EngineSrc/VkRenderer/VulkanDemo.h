#ifndef VULKAN_DEMO
#define VULKAN_DEMO

#include "Core/Global.h"
#include "Core/TimeStep.h"

#include "Pipeline.h"

namespace Gust 
{
class VulkanDemo 
{
public:
    VulkanDemo();

    void run(TimeStep timestep);

private:
    Pipeline pipeline;
};
}//GUST

#endif // !VULKAN_DEMO
