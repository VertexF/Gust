#include "Core/Logger.h"
#include "Core/Application.h"
#include "Core/Instrumentor.h"

#include <windows.h>

int main(int argc, char **argv) 
{
    Gust::Logger::init();
    GUST_PROFILE_BEGIN_SESSION("Startup", "GustProfile_startup.json");
    Gust::Application* app = new Gust::Application();
    GUST_PROFILE_END_SESSION();
    GUST_PROFILE_BEGIN_SESSION("runtime", "GustProfile_runtime.json");
    app->run();
    GUST_PROFILE_END_SESSION();
    GUST_PROFILE_BEGIN_SESSION("shutdown", "GustProfile_shutdown.json");
    delete app;
    GUST_PROFILE_END_SESSION();

    return 0;
}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) 
//{
//    Gust::Logger::init();
//    Gust::Application* app = new Gust::Application();
//    app->run();
//
//    return 0;
//}