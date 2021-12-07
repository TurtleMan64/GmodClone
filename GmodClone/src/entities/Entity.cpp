#include <glad/glad.h>

#include "entity.hpp"
#include "../models/models.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../main/main.hpp"
#include "../collision/collisionmodel.hpp"
#include "../models/models.hpp"

#include <list>
#include <iostream>
#include <string>

Entity::Entity()
{
    this->position.x = 0;
    this->position.y = 0;
    this->position.z = 0;
    this->rotX = 0;
    this->rotY = 0;
    this->rotZ = 0; 
    this->rotRoll = 0;
    this->scale = 1;
    this->visible = true;
    this->baseColor.set(1,1,1);
}

Entity::Entity(Vector3f* position, float rotX, float rotY, float rotZ, float scale)
{
    this->position.x = position->x;
    this->position.y = position->y;
    this->position.z = position->z;
    this->rotX = rotX;
    this->rotY = rotY;
    this->rotZ = rotZ;
    this->rotRoll = 0;
    this->scale = scale;
    this->visible = true;
    this->baseColor.set(1,1,1);
}

Entity::~Entity()
{

}

void Entity::step()
{

}

void Entity::die()
{
    
}

int Entity::getEntityType()
{
    return 0;
}

std::vector<Triangle3D*>* Entity::getCollisionTriangles()
{
    return nullptr;
}

void Entity::getHit(Vector3f* /*hitPos*/, Vector3f* /*hitDir*/, int /*weapon*/)
{

}

std::vector<Entity*>* Entity::getEntitiesToRender()
{
    return nullptr;
}

std::list<TexturedModel*>* Entity::getModels()
{
    return nullptr;
}

void Entity::setModelsRenderOrder(std::list<TexturedModel*>* models, char newOrder)
{
    for (TexturedModel* model : (*models))
    {
        model->renderOrder = newOrder;
    }
}

void Entity::updateTransformationMatrix()
{
    Maths::createTransformationMatrix(&transformationMatrix, &position, rotX, rotY, rotZ, rotRoll, scale);
}

void Entity::updateTransformationMatrixYXZY()
{
    Maths::createTransformationMatrixYXZY(&transformationMatrix, &position, rotX, rotY, rotZ, rotRoll, scale);
}

void Entity::updateTransformationMatrix(float scaleX, float scaleY, float scaleZ)
{
    Maths::createTransformationMatrix(&transformationMatrix, &position, rotX, rotY, rotZ, rotRoll, scaleX, scaleY, scaleZ);
}

void Entity::updateTransformationMatrixYXZ()
{
    Maths::createTransformationMatrixYXZ(&transformationMatrix, &position, rotX, rotY, rotZ, scale);
}

void Entity::deleteModels(std::list<TexturedModel*>* modelsToDelete)
{
    for (auto model : (*modelsToDelete))
    {
        model->deleteMe();
        delete model; INCR_DEL("TexturedModel");
    }
    modelsToDelete->clear();
}

void Entity::deleteCollisionModel(CollisionModel** colModelToDelete)
{
    if ((*colModelToDelete) != nullptr)
    {
        (*colModelToDelete)->deleteMe();
        delete (*colModelToDelete); INCR_DEL("CollisionModel");
        (*colModelToDelete) = nullptr;
    }
}
