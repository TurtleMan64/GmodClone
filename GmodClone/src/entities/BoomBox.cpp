#include <list>
#include <set>

#include "../toolbox/vector.hpp"
#include "boombox.hpp"
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

std::list<TexturedModel*> BoomBox::models;
CollisionModel* BoomBox::baseCM = nullptr;

extern float dt;

BoomBox::BoomBox(std::string name, Vector3f pos, float rot)
{
    this->name = name;

    cm = BoomBox::baseCM->duplicateMe();

    position = pos;
    visible = true;

    rotY = rot;

    entitiesToRender.push_back(this);

    baseCM->transformModel(cm, &position, rotY);

    updateTransformationMatrix();
}

BoomBox::~BoomBox()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }

    if (source != nullptr)
    {
        source->stop();
        source = nullptr;
    }
}

void BoomBox::step()
{
    if (source == nullptr)
    {
        source = AudioPlayer::play((int)(2*Maths::random()) + 64, &position, 1.0f, true);
    }
}

std::vector<Entity*>* BoomBox::getEntitiesToRender()
{
    return &entitiesToRender;
}

std::list<TexturedModel*>* BoomBox::getModels()
{
    return &BoomBox::models;
}

int BoomBox::getEntityType()
{
    return ENTITY_BOOM_BOX;
}

std::vector<Triangle3D*>* BoomBox::getCollisionTriangles()
{
    return &cm->triangles;
}

void BoomBox::loadModels()
{
    if (BoomBox::models.size() == 0)
    {
        ObjLoader::loadModel(&BoomBox::models, "res/Models/BoomBox/", "BoomBox");
        BoomBox::baseCM = ObjLoader::loadCollisionModel("res/Models/BoomBox/", "BoomBox");
    }
}