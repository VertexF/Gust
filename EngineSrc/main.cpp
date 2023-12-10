#include "Core/Logger.h"
#include "Core/Application.h"

#include <windows.h>

int main(int argc, char **argv) 
{
    Gust::Logger::init();
    Gust::Application* app = new Gust::Application();
    app->run();

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