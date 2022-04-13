#include <list>

#include "../toolbox/vector.hpp"
#include "healthcube.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../entities/entity.hpp"

Model HealthCube::model;

extern float dt;

HealthCube::HealthCube(std::string name, Vector3f pos)
{
    this->name = name;

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    updateTransformationMatrix();
}

void HealthCube::step()
{
    rotY += 240*dt;

    floatTimer += dt;

    float ogY = position.y;
    position.y = ogY + 0.125f*sinf(5*floatTimer) + 0.125f;
    updateTransformationMatrix();
    position.y = ogY;
}

std::vector<Entity*>* HealthCube::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* HealthCube::getModel()
{
    return &HealthCube::model;
}

int HealthCube::getEntityType()
{
    return ENTITY_HEALTH_CUBE;
}

void HealthCube::loadModels()
{
    if (!HealthCube::model.isLoaded())
    {
        ObjLoader::loadModel(&HealthCube::model, "res/Models/HealthCube/", "HealthCube");
    }
}
