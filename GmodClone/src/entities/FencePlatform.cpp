#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "fenceplatform.hpp"
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
#include "../audio/audioplayer.hpp"
#include "../network/tcpclient.hpp"

std::list<TexturedModel*> FencePlatform::models;
CollisionModel* FencePlatform::baseCM = nullptr;

extern float dt;

FencePlatform::FencePlatform(std::string name, Vector3f pos)
{
    this->name = name;

    cm = FencePlatform::baseCM->duplicateMe();

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    baseCM->transformModel(cm, &position, 0);

    updateTransformationMatrix();
}

FencePlatform::~FencePlatform()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }
}

void FencePlatform::step()
{
    if (Global::timeUntilRoundStarts > 0.0f)
    {
        visible = true;
    }
    else
    {
        visible = false;
    }
}

std::vector<Entity*>* FencePlatform::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* FencePlatform::getModels()
{
    return &FencePlatform::models;
}

int FencePlatform::getEntityType()
{
    return ENTITY_FENCE_PLATFORM;
}

std::vector<Triangle3D*>* FencePlatform::getCollisionTriangles()
{
    if (Global::timeUntilRoundStarts > 0.0f)
    {
        return &cm->triangles;
    }

    return nullptr;
}

void FencePlatform::loadModels()
{
    if (FencePlatform::models.size() == 0)
    {
        ObjLoader::loadModel(&FencePlatform::models, "res/Models/FencePlatform/", "FencePlatform");
        FencePlatform::baseCM = ObjLoader::loadCollisionModel("res/Models/FencePlatform/", "FencePlatform");
    }
}
