#include "Core/Logger.h"
#include "Core/Application.h"

//TODO: Move file out of Core

int main() 
{
    Gust::Logger::init();
    GUST_CRITICAL("The Engine");
    GUST_ERROR("The Engine");
    GUST_WARN("The Engine");
    GUST_INFO("The Engine");
    GUST_TRACE("The Engine");

    Gust::Application* app = new Gust::Application();
    app->run();

    return 0;
}