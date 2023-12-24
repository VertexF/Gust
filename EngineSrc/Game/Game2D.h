#ifndef GAME_2D_HDR
#define GAME_2D_HDR

#include "Entity/GameObject.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Core/TimeStep.h"
#include "Core/Layer.h"

#include "VkRenderer/VulkanDemo.h"

namespace game 
{

class Game2D : public Gust::Layer
{
public:
    Game2D();
    virtual ~Game2D() = default;

    virtual void attach() override;
    virtual void detach() override;
    virtual void update(Gust::TimeStep timeStep) override;
    virtual void handleEvent(Gust::Event& ent) override;

    bool keyPressed(Gust::PressedKeyEvent& ent);
    bool keyReleased(Gust::ReleasedKeyEvent& ent);
    bool mouseReleased(Gust::MouseButtonReleasedEvent& ent);
private:
    void loadGameObjects();

    Gust::VulkanDemo* _vulkanDemo;

    std::unordered_map<uint32_t, Gust::GameObject> _gameObjects;

    int _mouseButton = 0;
};

}

#endif // !GAME_2D_HDR
