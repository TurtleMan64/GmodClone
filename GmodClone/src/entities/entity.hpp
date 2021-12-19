#ifndef ENTITIES_H
#define ENTITIES_H

class TexturedModel;
class CollisionModel;
class Triangle3D;

#include <list>
#include <vector>
#include "../toolbox/vector.hpp"
#include "../toolbox/matrix.hpp"
#include <string>

#define ENTITY_BLANK 0
#define ENTITY_NPC 1
#define ENTITY_BALL 2
#define ENTITY_ONLINE_PLAYER 3
#define ENTITY_COLLISION_BLOCK 4
#define ENTITY_RED_BARREL 5
#define ENTITY_LADDER 6
#define ENTITY_HEALTH_CUBE 7
#define ENTITY_GLASS 8
#define ENTITY_BOOM_BOX 9
#define ENTITY_ROCK_PLATFORM 10
#define ENTITY_CHANDELIER 11
#define ENTITY_FENCE_PLATFORM 12
#define ENTITY_STEP_FALL_PLATFORM 13
#define ENTITY_BAT 14
#define ENTITY_WIN_ZONE 15

#define WEAPON_FIST 0
#define WEAPON_BAT 1
#define WEAPON_GUN 2


class Entity
{
public:
    std::string name = "Default"; //Name will be Human1, Ball1, Human2, etc.
    Vector3f position;
    Vector3f vel;
    float rotX    = 0.0f;
    float rotY    = 0.0f;
    float rotZ    = 0.0f;
    float rotRoll = 0.0f;
    float scale   = 1.0f;
    bool visible  = true;
    Vector3f baseColor;
    float baseAlpha = 1.0f;
    Matrix4f transformationMatrix;
    //render order is normally set in each TexturedModel, but can be overrided by using this
    char renderOrderOverride = 69;

    std::vector<Entity*> entitiesToRender;

    static void deleteModels(std::list<TexturedModel*>* modelsToDelete);
    static void deleteCollisionModel(CollisionModel** colModelToDelete);
    //0 = rendered first (default), 1 = second, 2 = third, 3 = fourth + transparent (no depth testing)
    static void setModelsRenderOrder(std::list<TexturedModel*>* models, char newOrder);

public:
    Entity();
    Entity(Vector3f* initialPosition, float rotX, float rotY, float rotZ, float scale);
    virtual ~Entity();

    virtual void step();

    virtual std::vector<Entity*>* getEntitiesToRender();

    virtual std::list<TexturedModel*>* getModels();

    void updateTransformationMatrix();

    void updateTransformationMatrixYXZY();

    void updateTransformationMatrix(float scaleX, float scaleY, float scaleZ);

    void updateTransformationMatrixYXZ();

    virtual void die();

    virtual int getEntityType();

    virtual std::vector<Triangle3D*>* getCollisionTriangles();

    virtual void getHit(Vector3f* hitPos, Vector3f* hitDir, int weapon);
};
#endif
