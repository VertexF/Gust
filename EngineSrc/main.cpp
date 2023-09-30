//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZEOR_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <stb_image.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <stb_image.h>
#include <stb_truetype.h>
#include <tiny_obj_loader.h>

#include <iostream>

int main() 
{
    std::string title = "Gust Engine";
    int width = 800;
    int height = 600;

    if (glfwInit() == false)
    {
        std::cout << "GLFW could not start!\n";
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    //Optional setup but good for optimiation
    VkApplicationInfo appInfo{};
    //These sType's define the type of struct we are setting up.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Game Engine";
    //Setup version
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Gust";
    //Setup version
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    return 0;
}