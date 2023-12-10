#ifndef GLOBAL_HDR
#define GLOBAL_HDR

//Needed to make the GLEW linking is linking to the static version.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan.h"

#include "Logger.h"

class Global
{
public:
    static Global& getInstance()
    {
        static Global instance;
        return instance;
    }

    //TODO: Add the global access to GLFWwindow here singleton style.
    GLFWwindow* getWindow() const
    {
        GUST_CORE_ASSERT(_window == nullptr, "The window is null, you need to set the GLFWwindow before you use it.")
        return _window; 
    }

    void setWindow(GLFWwindow *window) 
    {
        _window = window;
    }

    void setWindowExtent(int width, int height) 
    {
        _windowExtent.width = static_cast<uint32_t>(width);
        _windowExtent.height = static_cast<uint32_t>(height);
    }

    VkExtent2D getWindowExtent() const 
    {
        return _windowExtent;
    }


    bool running;
private:
    Global() : running(true), _window(nullptr)
    {
    }

    ~Global() 
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    GLFWwindow* _window;
    VkExtent2D _windowExtent;

public:
    Global(Global const& rhs) = delete;
    Global(Global && rhs) = delete;
    void operator=(Global const& rhs) = delete;
    Global operator=(Global &&rhs) = delete;
};

#endif // !GLOBAL_HDR
