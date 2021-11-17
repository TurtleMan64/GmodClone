#ifndef LADDER_H
#define LADDER_H

#include <vector>

#include "entity.hpp"
#include "../toolbox/vector.hpp"
#include "../collision/collisionmodel.hpp"

class Triangle3D;

class Ladder : public Entity
{
public:
    // Radius of each dimension of rectangular prism
    Vector3f size;
    CollisionModel* cm = nullptr;

    Ladder(std::string name, Vector3f pos, Vector3f size);
    ~Ladder();

    void step();

    int getEntityType();
};
#endif
