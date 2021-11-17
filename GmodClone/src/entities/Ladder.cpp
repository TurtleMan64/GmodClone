#include <vector>

#include "../toolbox/vector.hpp"
#include "ladder.hpp"
#include "../main/main.hpp"
#include "../entities/entity.hpp"
#include "../collision/triangle3d.hpp"
#include "../collision/collisionmodel.hpp"
#include "../loader/objloader.hpp"

extern float dt;

Ladder::Ladder(std::string name, Vector3f pos, Vector3f size)
{
    this->name = name;

    position = pos;
    this->size = size;
    visible = false;

    updateTransformationMatrix(size.x, size.y, size.z);

    CollisionModel* baseCM = ObjLoader::loadCollisionModel("Models/Ladder/", "Ladder");

    cm = baseCM->duplicateMe();

    baseCM->transformModelWithScale(cm, &position, &size);
    
    baseCM->deleteMe();
    delete baseCM;
}

Ladder::~Ladder()
{
    if (cm != nullptr)
    {
        cm->deleteMe();
        delete cm;
    }
}

void Ladder::step()
{

}

int Ladder::getEntityType()
{
    return ENTITY_LADDER;
}
