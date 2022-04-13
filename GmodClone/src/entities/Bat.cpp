#include <list>
#include <vector>

#include "../toolbox/vector.hpp"
#include "bat.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../entities/entity.hpp"

Model Bat::model;

extern float dt;

Bat::Bat(std::string name, Vector3f pos)
{
    this->name = name;

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    updateTransformationMatrix();
}

void Bat::step()
{
    rotY += 240*dt;

    floatTimer += dt;

    float ogY = position.y;
    position.y = ogY + 0.125f*sinf(5*floatTimer) + 0.125f;
    updateTransformationMatrix();
    position.y = ogY;
}

std::vector<Entity*>* Bat::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* Bat::getModel()
{
    return &Bat::model;
}

int Bat::getEntityType()
{
    return ENTITY_BAT;
}

void Bat::loadModels()
{
    if (!Bat::model.isLoaded())
    {
        ObjLoader::loadModel(&Bat::model, "res/Models/Bat/", "Display");
    }
}
