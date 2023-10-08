#include "Core/Logger.h"
#include "Core/Application.h"

int main() 
{
    Gust::Logger::init();
    Gust::Application* app = new Gust::Application();
    app->run();

    return 0;
}