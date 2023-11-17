#include "StaticEntity.h"

namespace Gust 
{
Entity::Entity() : _mesh(nullptr), _material(nullptr), _transformMatrix(glm::mat4(1.f))
{
}

void Entity::setMesh(Mesh* mesh) 
{
    _mesh = mesh;
}

Mesh* Entity::getMesh() const 
{
    return _mesh;
}

void Entity::setMaterial(Material* material) 
{
    _material = material;
}

Material* Entity::getMaterial() const 
{
    return _material;
}

void Entity::setTransform(const glm::mat4& transform) 
{
    _transformMatrix = transform;
}

glm::mat4 Entity::getTransform() const 
{
    return _transformMatrix;
}

}