#include "Game2D.h"

#include "Core/Logger.h"

namespace game 
{
Game2D::Game2D() : Gust::Layer("Game World")
{
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
    //TODO: Update and render.
}

void Game2D::handleEvent(Gust::Event& ent) 
{
    //Handle user input TODO: Handle at least the Esc key.
    Gust::EventDispatcher dispatcher(ent);
    dispatcher.dispatch<Gust::PressedKeyEvent>(std::bind(&Game2D::keyPressed, this, std::placeholders::_1));
    dispatcher.dispatch<Gust::ReleasedKeyEvent>(std::bind(&Game2D::keyReleased, this, std::placeholders::_1));
    dispatcher.dispatch<Gust::MouseButtonReleasedEvent>(std::bind(&Game2D::mouseReleased, this, std::placeholders::_1));
}

bool Game2D::keyPressed(Gust::PressedKeyEvent& ent) 
{
    return false;
}

bool Game2D::keyReleased(Gust::ReleasedKeyEvent& ent) 
{
    GUST_INFO("Keycode is: {0}", ent.getKeyCode());
    return false;
}

bool Game2D::mouseReleased(Gust::MouseButtonReleasedEvent& ent) 
{
    GUST_INFO("Mouse button is: {0}", ent.getMouseButton());
    //One this event get mouse position.
    return false;
}

} //GUST