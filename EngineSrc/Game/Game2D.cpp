#include "Game2D.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Logger.h"
#include "Core/KeyCodes.h"
#include "Core/Global.h"
#include "Core/Instrumentor.h"

#include "VkRenderer/Model.h"

namespace game 
{
Game2D::Game2D() : Gust::Layer("Game World")
{
    GUST_PROFILE_FUNCTION();
    _vulkanDemo = new Gust::VulkanDemo();
}

void Game2D::attach() 
{
    GUST_PROFILE_FUNCTION();
    loadGameObjects();
}
void Game2D::detach() 
{
    //Do clean up
}

void Game2D::update(Gust::TimeStep timeStep) 
{
    GUST_PROFILE_FUNCTION();
    _vulkanDemo->renderer(timeStep, _gameObjects);
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
    return false;
}

bool Game2D::keyReleased(Gust::ReleasedKeyEvent& ent)
{
    if (ent.getKeyCode() == GUST_KEY_ESCAPE)
    {
        Global::getInstance().running = false;
    }

    if (ent.getKeyCode() == GUST_KEY_SPACE) 
    {
        _gameObjects.at(2).currentState = Gust::State::ACTIVE;
    }
    return false;
}

bool Game2D::mouseReleased(Gust::MouseButtonReleasedEvent& ent) 
{
    GUST_INFO("Mouse button is: {0}", ent.getMouseButton());
    //One this event get mouse position.

    _mouseButton = ent.getMouseButton();
    return false;
}

void Game2D::loadGameObjects()
{
    GUST_PROFILE_FUNCTION();
    Gust::GameObject smoothVase = Gust::GameObject::createGameObject();
    smoothVase.model = Gust::Model::createModelFromFile("Assets/Models/smooth_vase.obj");
    smoothVase.transform.translation = { 0.5f, 0.5f, 0.f };
    smoothVase.transform.scale = glm::vec3(1.5f, 3.f, 1.5f);

    Gust::GameObject flatVase = Gust::GameObject::createGameObject();
    flatVase.model = Gust::Model::createModelFromFile("Assets/Models/flat_vase.obj");
    flatVase.transform.translation = { -0.5f, 0.5f, 0.f };
    flatVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
    flatVase.currentState = 0;

    Gust::GameObject floor = Gust::GameObject::createGameObject();
    floor.model = Gust::Model::createModelFromFile("Assets/Models/quad.obj");
    floor.transform.translation = { 0.f, 0.5f, 0.f };
    floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);

    std::vector<glm::vec3> lightColours =
    {
        { 1.f, 0.f, 0.f },
        { 0.f, 0.f, 1.f },
        { 0.f, 1.f, 0.f },
        { 1.f,  1.f, 0.f },
        { 0.f, 1.f, 1.f },
        { 1.f,  1.f, 1.f }
    };

    for (int i = 0; i < lightColours.size(); i++)
    {
        auto pointLight = Gust::GameObject::makePointLight(0.3f);
        pointLight.colour = lightColours[i];
        auto rotateLight = glm::rotate
        (
            glm::mat4(1.f),
            (i * glm::two_pi<float>()) / lightColours.size(),
            { 0.f, -1.f, 0.f }
        );

        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.0f, -0.5f, -1.0f, 1.f));
        _gameObjects.emplace(pointLight.getID(), std::move(pointLight));
    }

    _gameObjects.emplace(smoothVase.getID(), std::move(smoothVase));
    _gameObjects.emplace(flatVase.getID(), std::move(flatVase));
    _gameObjects.emplace(floor.getID(), std::move(floor));
}

} //GUST