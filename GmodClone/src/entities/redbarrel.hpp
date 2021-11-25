#ifndef REDBARREL_H
#define REDBARREL_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class RedBarrel : public Entity
{
private:
    static std::list<TexturedModel*> models;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

    float health = 1.0f;

public:
    RedBarrel(std::string name, Vector3f pos);

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();

    void getHit(Vector3f* hitPos, Vector3f* hitDir, int weapon);
};
#endif
