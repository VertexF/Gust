#include "VulkanDemo.h"

#include "Core/Logger.h"


namespace Gust 
{

VulkanDemo::VulkanDemo() : pipeline("Assets/Shaders/simple.vert.spv", "Assets/Shaders/simple.frag.spv")
{
}

void VulkanDemo::run(TimeStep timestep)
{
}

}//GUST