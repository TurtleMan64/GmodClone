#include <vector>

#include "../toolbox/vector.hpp"
#include "ladder.hpp"
#include "../main/main.hpp"
#include "../entities/entity.hpp"
#include "../collision/triangle3d.hpp"
#include "../collision/collisionmodel.hpp"
#include "../loader/objloader.hpp"

extern float dt;

CollisionModel* Ladder::cmBase = nullptr;

Ladder::Ladder(std::string name, Vector3f pos, Vector3f size)
{
    this->name = name;

    position = pos;
    this->size = size;
    visible = false;

    updateTransformationMatrix(size.x, size.y, size.z);

    cm = Ladder::cmBase->duplicateMe();

    Ladder::cmBase->transformModelWithScale(cm, &position, &size);
}

Ladder::~Ladder()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm; INCR_DEL("CollisionModel");
    }
}

void Ladder::step()
{

}

int Ladder::getEntityType()
{
    return ENTITY_LADDER;
}

void Ladder::loadModels()
{
    if (Ladder::cmBase == nullptr)
    {
        Ladder::cmBase = ObjLoader::loadCollisionModel("res/Models/Ladder/", "Ladder");
    }
}
