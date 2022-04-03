#include <list>
#include <set>
#include <cstring>

#include "../toolbox/vector.hpp"
#include "glass.hpp"
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

std::list<TexturedModel*> Glass::models;
CollisionModel* Glass::baseCM = nullptr;

extern float dt;

Glass::Glass(std::string name, Vector3f pos)
{
    this->name = name;

    cm = Glass::baseCM->duplicateMe();

    position = pos;
    visible = true;

    entitiesToRender.push_back(this);

    baseCM->transformModel(cm, &position, 0);

    updateTransformationMatrix();
}

Glass::~Glass()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }
}

void Glass::step()
{
    if (hasBroken)
    {
        visible = false;
    }
    else
    {
        visible = true;
    }

    if (!hasBroken && !isReal)
    {
        //if (!Global::serverClient->isOpen())
        {
            if (Global::player->health <= 0)
            {
                return;
            }

            Vector3f playerCenter = Global::player->position;
            const float COLLISION_RADIUS = 1.74f/4;
            playerCenter.y += COLLISION_RADIUS;

            for (Triangle3D* tri : cm->triangles)
            {
                float d;
                Vector3f g;
                if (Maths::sphereIntersectsTriangle(&playerCenter, COLLISION_RADIUS, tri, &d, &g))
                {
                    AudioPlayer::play(62, nullptr);
                    Global::sendAudioMessageToServer(62, &position);
                    hasBroken = true;

                    if (Global::serverClient->isOpen())
                    {
                        int nameLen = (int)name.size();

                        Message msg;
                        msg.length = 5 + nameLen + 2;

                        msg.buf[0] = 8;
                        memcpy(&msg.buf[1], &nameLen, 4);
                        memcpy(&msg.buf[5], name.c_str(), nameLen);
                        msg.buf[5 + nameLen    ] = (char)isReal;
                        msg.buf[5 + nameLen + 1] = (char)hasBroken;

                        Global::sendMessageToServer(msg);
                    }
                }
            }
        }
    }
}

std::vector<Entity*>* Glass::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* Glass::getModels()
{
    return &Glass::models;
}

int Glass::getEntityType()
{
    return ENTITY_GLASS;
}

std::vector<Triangle3D*>* Glass::getCollisionTriangles()
{
    if (!hasBroken && isReal)
    {
        return &cm->triangles;
    }

    return nullptr;
}

void Glass::loadModels()
{
    if (Glass::models.size() == 0)
    {
        ObjLoader::loadModel(&Glass::models, "res/Models/Glass/", "Glass");
        Glass::baseCM = ObjLoader::loadCollisionModel("res/Models/Glass/", "Glass");
    }
}
