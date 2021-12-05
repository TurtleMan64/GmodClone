#ifndef FENCEPLATFORM_H
#define FENCEPLATFORM_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class FencePlatform : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

public:
    FencePlatform(std::string name, Vector3f pos);
    ~FencePlatform();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
