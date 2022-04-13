#ifndef BOOMBOX_H
#define BOOMBOX_H

class TexturedModel;
class Triangle3D;
class CollisionModel;
class Source;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class BoomBox : public Entity
{
private:
    static Model models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

    Source* source = nullptr;

public:
    BoomBox(std::string name, Vector3f pos, float rot);
    ~BoomBox();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
