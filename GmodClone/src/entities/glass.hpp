#ifndef GLASS_H
#define GLASS_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"
#include "../audio/audioplayer.hpp"

class Glass : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

public:
    bool isReal = true;
    bool hasBroken = false;

    Glass(std::string name, Vector3f pos);
    ~Glass();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
