#include "Game2D.h"

#include "Core/Logger.h"
#include "Core/KeyCodes.h"
#include "Core/Global.h"

namespace game 
{
Game2D::Game2D() : Gust::Layer("Game World")
{
    _vulkan = new Gust::Vulkan("Gust Engine");
}

void Game2D::attach() 
{
    //Load assets
}
void Game2D::detach() 
{
    //Do clean up
}

void Game2D::update(Gust::TimeStep timeStep) 
{
    _vulkan->drawFrame(timeStep);
}

void Game2D::handleEvent(Gust::Event& ent) 
{
    Gust::EventDispatcher dispatcher(ent);
    dispatcher.dispatch<Gust::PressedKeyEvent>(std::bind(&Game2D::keyPressed, this, std::placeholders::_1));
    dispatcher.dispatch<Gust::ReleasedKeyEvent>(std::bind(&Game2D::keyReleased, this, std::placeholders::_1));
    dispatcher.dispatch<Gust::MouseButtonReleasedEvent>(std::bind(&Game2D::mouseReleased, this, std::placeholders::_1));
}

bool Game2D::keyPressed(Gust::PressedKeyEvent& ent) 
{
    GUST_INFO("Keycode is: {0}", ent.getKeyCode());
    return false;
}

bool Game2D::keyReleased(Gust::ReleasedKeyEvent& ent) 
{
    GUST_INFO("Keycode is: {0}", ent.getKeyCode());
    if (ent.getKeyCode() == GUST_KEY_ESCAPE)
    {
        Global::getInstance().running = false;
    }
    return false;
}

bool Game2D::mouseReleased(Gust::MouseButtonReleasedEvent& ent) 
{
    GUST_INFO("Mouse button is: {0}", ent.getMouseButton());
    //One this event get mouse position.
    return false;
}

} //GUST