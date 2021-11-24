#include <list>
#include <set>

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

std::list<TexturedModel*> Glass::models;
CollisionModel* Glass::baseCM = nullptr;

extern float dt;

Glass::Glass(std::string name, Vector3f pos)
{
    this->name = name;

    cm = Glass::baseCM->duplicateMe();

    position = pos;
    visible = true;

    baseCM->transformModel(cm, &position, 0);

    updateTransformationMatrix();
}

void Glass::step()
{
    
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
        Glass::baseCM = ObjLoader::loadCollisionModel("Models/Glass/", "Glass");
    }
}
