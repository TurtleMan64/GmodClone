#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "collisionblock.hpp"
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

std::list<TexturedModel*> CollisionBlock::models;
CollisionModel* CollisionBlock::baseCM = nullptr;

extern float dt;

CollisionBlock::CollisionBlock(std::string name, Vector3f pos, int type)
{
    this->name = name;
    this->type = type;

    if (CollisionBlock::models.size() == 0)
    {
        ObjLoader::loadModel(&CollisionBlock::models, "res/Models/Cheese/", "Cheese");
        CollisionBlock::baseCM = ObjLoader::loadCollisionModel("Models/Cheese/", "Cheese");
    }

    cm = CollisionBlock::baseCM->duplicateMe();

    scale = 5.5f;

    position = pos;
    startPos = pos;
    visible = true;
}

void CollisionBlock::step()
{
    if (type == 0)
    {
        position.y = startPos.y + 5.0f*sinf((float)Global::syncedGlobalTime);

        if (Global::player->collideEntityImTouching == this && 
            Global::player->position.y >= (position.y + scale) - 0.01f)
        {
            Global::player->externalVel.set(0, 5.0f*cosf((float)Global::syncedGlobalTime), 0);
        }
        //if (fmodf((float)Global::syncedGlobalTime, 3.0f) < 1.5f)
        //{
        //    if (Global::player->collideEntityImTouching == this && 
        //        Global::player->position.y >= (position.y + scale) - 0.01f)
        //    {
        //        //printf("running\n");
        //        //Global::player->vel.y = 5.0f;
        //        Global::player->externalVel.set(0, 5, 0);
        //        //Global::player->position.y += 1.0f;
        //    }
        //
        //    position.y += 5*dt;
        //}
        //else
        //{
        //    position.y -= 5*dt;
        //
        //    if (Global::player->collideEntityImTouching == this && 
        //        Global::player->position.y > position.y + scale)
        //    {
        //        //Global::player->vel.y += -500*dt;
        //    }
        //}
    }
    else if (type == 1)
    {
        position.x = startPos.x + 2.0f*sinf((float)Global::syncedGlobalTime);

        if (Global::player->collideEntityImTouching == this && 
            Global::player->position.y >= (position.y + scale) - 0.01f)
        {
            Global::player->externalVel.set(2.0f*cosf((float)Global::syncedGlobalTime), 0, 0);
        }
    }

    baseCM->transformModelWithScale(cm, &position, 0, scale);

    updateTransformationMatrix();
}

std::list<TexturedModel*>* CollisionBlock::getModels()
{
    return &CollisionBlock::models;
}

int CollisionBlock::getEntityType()
{
    return ENTITY_COLLISION_BLOCK;
}

std::vector<Triangle3D*>* CollisionBlock::getCollisionTriangles()
{
    return &cm->triangles;
}
