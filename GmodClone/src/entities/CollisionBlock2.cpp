#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "collisionblock2.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/collisionmodel.hpp"
#include "../collision/triangle3d.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../audio/audioplayer.hpp"
#include "../audio/source.hpp"
#include "../entities/entity.hpp"
#include "../entities/player.hpp"
#include "../entities/onlineplayer.hpp"

Model CollisionBlock2::model;
CollisionModel* CollisionBlock2::baseCM = nullptr;

extern float dt;

CollisionBlock2::CollisionBlock2(std::string name, Vector3f pos, float radius, float width, float height, float timeForOneRotation, float startAngle)
{
    this->name = name;
    this->radius = radius;
    this->width = width;
    this->height = height;
    this->timeForOneRotation = timeForOneRotation;
    this->startAngle = startAngle;
    this->scale = 1;

    cm = CollisionBlock2::baseCM->duplicateMe();

    position = pos;
    startPos = pos;
    visible = true;

    entitiesToRender.push_back(this);
}

CollisionBlock2::~CollisionBlock2()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
        cm = nullptr;
    }
}

void CollisionBlock2::step()
{
    rotY = (float)(360*(Global::syncedGlobalTime/timeForOneRotation) + startAngle);

    baseCM->transformModelWithScale(cm, &position, rotY, radius, height, width);
    updateTransformationMatrix(radius, height, width);

    if (Global::player->health <= 0)
    {
        return;
    }
}

std::vector<Entity*>* CollisionBlock2::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* CollisionBlock2::getModel()
{
    return &CollisionBlock2::model;
}

int CollisionBlock2::getEntityType()
{
    return ENTITY_COLLISION_BLOCK_2;
}

std::vector<Triangle3D*>* CollisionBlock2::getCollisionTriangles()
{
    return &cm->triangles;
}

void CollisionBlock2::loadModels()
{
    if (!CollisionBlock2::model.isLoaded())
    {
        ObjLoader::loadModel(&CollisionBlock2::model, "res/Models/CollisionBlock/", "CollisionBlock2");
        CollisionBlock2::baseCM = ObjLoader::loadCollisionModel("res/Models/CollisionBlock/", "CollisionBlock2");
    }
}
