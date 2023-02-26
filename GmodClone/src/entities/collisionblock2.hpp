#ifndef COLLISIONBLOCK2_H
#define COLLISIONBLOCK2_H

class TexturedModel;
class Triangle3D;
class CollisionModel;

#include <list>
#include <vector>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class CollisionBlock2 : public Entity
{
private:
    static Model model;
    static CollisionModel* baseCM;

    CollisionModel* cm = nullptr;

    Vector3f startPos;

    float radius;
    float width;
    float height;
    float timeForOneRotation;
    float startAngle;

public:
    //direction: 0 = x axis, 1 = y axis, 2 = z axis
    //radius: radius of block
    //timePeriod: time for a full cycle
    //distance: distance traveled in the full cycle
    //sinusoidal: false for linear
    CollisionBlock2(std::string name, Vector3f pos, float radius, float width, float height, float timeForOneRotation, float startAngle);
    ~CollisionBlock2();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModel();

    static void loadModels();

    int getEntityType();

    std::vector<Triangle3D*>* getCollisionTriangles();
};
#endif
