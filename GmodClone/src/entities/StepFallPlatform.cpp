#include <list>
#include <set>
#include <cstring>

#include "../toolbox/vector.hpp"
#include "stepfallplatform.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/collisionmodel.hpp"
#include "../collision/triangle3d.hpp"
#include "../main/main.hpp"
#include "../toolbox/maths.hpp"
#include "../loader/objloader.hpp"
#include "../entities/entity.hpp"
#include "../entities/player.hpp"
#include "../network/tcpclient.hpp"

Model StepFallPlatform::model;
CollisionModel* StepFallPlatform::baseCM = nullptr;

extern float dt;

StepFallPlatform::StepFallPlatform(std::string name, Vector3f pos)
{
    this->name = name;

    cm = StepFallPlatform::baseCM->duplicateMe();

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    baseCM->transformModel(cm, &position, 0);

    scale = 0.975f;

    updateTransformationMatrix();
}

StepFallPlatform::~StepFallPlatform()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }
}

void StepFallPlatform::step()
{
    if (timeUntilBreaks <= 0.0f)
    {
        visible = false;
    }
    else
    {
        visible = true;
    }

    if (Global::timeUntilRoundEnds > 0.0f)
    {
        timeUntilBreaks -= dt;

        if (timeUntilBreaks > 100.0f)
        {
            if (Global::player->health > 0 && Global::player->collideEntityImTouching == this)
            {
                timeUntilBreaks = 2.0f;

                Message msg;
                msg.buf[0] = 14;

                int nameLen = (int)name.size();
                memcpy(&msg.buf[1], &nameLen, 4);
                memcpy(&msg.buf[5], name.c_str(), nameLen);

                msg.length = 1 + 4 + nameLen;

                Global::sendMessageToServer(msg);
            }
        }

        if (timeUntilBreaks < 2.0f && timeUntilBreaks > 0.0f)
        {
            baseColor.set(timeUntilBreaks/2.0f, timeUntilBreaks/2.0f, timeUntilBreaks/2.0f);
        }
    }
}

std::vector<Entity*>* StepFallPlatform::getEntitiesToRender()
{
    return &entitiesToRender;
}

Model* StepFallPlatform::getModel()
{
    return &StepFallPlatform::model;
}

int StepFallPlatform::getEntityType()
{
    return ENTITY_STEP_FALL_PLATFORM;
}

std::vector<Triangle3D*>* StepFallPlatform::getCollisionTriangles()
{
    if (timeUntilBreaks > 0.0f)
    {
        return &cm->triangles;
    }

    return nullptr;
}

void StepFallPlatform::loadModels()
{
    if (!StepFallPlatform::model.isLoaded())
    {
        ObjLoader::loadModel(&StepFallPlatform::model, "res/Models/StepFallPlatform/", "StepFallPlatform");
        StepFallPlatform::baseCM = ObjLoader::loadCollisionModel("res/Models/StepFallPlatform/", "Collision");
    }
}
