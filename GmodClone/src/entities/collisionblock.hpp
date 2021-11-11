#ifndef COLLISIONBLOCK_H
#define COLLISIONBLOCK_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class CollisionBlock : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

    Vector3f startPos;

    int type = 0;

public:
    CollisionBlock(std::string name, Vector3f pos, int type);

    void step();

    std::list<TexturedModel*>* getModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
