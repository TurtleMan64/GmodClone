#ifndef ROCKPLATFORM_H
#define ROCKPLATFORM_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"
#include "../audio/audioplayer.hpp"

class RockPlatform : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

public:
    float timeUntilBreaks = 100000000.0f;

    RockPlatform(std::string name, Vector3f pos);
    ~RockPlatform();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
