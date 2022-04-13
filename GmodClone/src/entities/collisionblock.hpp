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
    static Model model;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

    Vector3f startPos;

    int direction = 0;
    float timePeriod = 1.0f;
    float distance = 1.0f;
    bool sinusoidal = false;
    float timeOffset = 0.0f;



public:
    //direction: 0 = x axis, 1 = y axis, 2 = z axis
    //radius: radius of block
    //timePeriod: time for a full cycle
    //distance: distance traveled in the full cycle
    //sinusoidal: false for linear
    CollisionBlock(std::string name, Vector3f pos, int direction, float radius, float timePeriod, float distance, bool sinusoidal, float timeOffset);
    ~CollisionBlock();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModel();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
