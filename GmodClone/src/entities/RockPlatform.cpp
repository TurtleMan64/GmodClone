#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "rockplatform.hpp"
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

std::list<TexturedModel*> RockPlatform::models;
CollisionModel* RockPlatform::baseCM = nullptr;

extern float dt;

RockPlatform::RockPlatform(std::string name, Vector3f pos)
{
    this->name = name;

    cm = RockPlatform::baseCM->duplicateMe();

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    baseCM->transformModel(cm, &position, 0);

    updateTransformationMatrix();
}

RockPlatform::~RockPlatform()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }
}

void RockPlatform::step()
{
    if (timeUntilBreaks <= 0.0f)
    {
        visible = false;
    }
    else
    {
        visible = true;
    }

    if (Global::timeUntilRoundStarts <= 0.0f)
    {
        timeUntilBreaks -= dt;
    }

    if (timeUntilBreaks < 3.0f && timeUntilBreaks > 0.0f)
    {
        float origY = position.y;
        position.y += 0.1f*Maths::random() - 0.05f;

        updateTransformationMatrix();

        position.y = origY;
    }
}

std::vector<Entity*>* RockPlatform::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* RockPlatform::getModels()
{
    return &RockPlatform::models;
}

int RockPlatform::getEntityType()
{
    return ENTITY_ROCK_PLATFORM;
}

std::vector<Triangle3D*>* RockPlatform::getCollisionTriangles()
{
    if (timeUntilBreaks > 0.0f)
    {
        return &cm->triangles;
    }

    return nullptr;
}

void RockPlatform::loadModels()
{
    if (RockPlatform::models.size() == 0)
    {
        ObjLoader::loadModel(&RockPlatform::models, "res/Models/RockPlatform/", "RockPlatform");
        RockPlatform::baseCM = ObjLoader::loadCollisionModel("res/Models/RockPlatform/", "RockPlatform");
    }
}
