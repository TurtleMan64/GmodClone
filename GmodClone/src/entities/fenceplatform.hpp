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
    static Model model;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

public:
    FencePlatform(std::string name, Vector3f pos);
    ~FencePlatform();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModel();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
