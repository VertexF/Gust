#include "GameObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Gust 
{
GameObject::GameObject(uint32_t objectID) : 
    _objectID(objectID), model(nullptr), pointLight(nullptr), colour(glm::vec3(1.f)), currentState(State::ACTIVE)
{
}

GameObject GameObject::createGameObject()
{
    static uint32_t currentID = 0;
    return GameObject(currentID++);
}

int GameObject::getID() 
{ 
    return _objectID; 
}

GameObject GameObject::makePointLight(float intensity /*= 10.f*/, float radius /*= 0.1f*/, 
                                      glm::vec3 colour /*= glm::vec3(1.f)*/) 
{
    GameObject gameObject = createGameObject();
    gameObject.colour = colour;
    gameObject.transform.scale.x = radius;
    gameObject.pointLight = new PointLightComponent();
    gameObject.pointLight->lightIntensity = intensity;
    gameObject.model = nullptr;

    return gameObject;
}

}//Gust