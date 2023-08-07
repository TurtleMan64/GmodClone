#ifndef BALL_H
#define BALL_H

class TexturedModel;
class Source;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"
#include "../collision/collisionchecker.hpp"

class Ball : public Entity
{
private:
    static Model model;

    Source* src = nullptr;

    // Vars to be used after the moving blocks have moved.
    CollisionResult resultPrev;
    Vector3f baryCoordsPrev;
    Vector3f velPrev;

    std::vector<float> rotYs;
    int currentRotY = 0;

    const float gravityForce = 0.588f*60;

    const float DRAG_AIR = 0.4f;

    const float bounceAmount = 0.85f; //85% of relative vertical speed retain after bounce

public:
    std::unordered_set<Ball*> ballsCollidedWith;

    Ball(std::string name, Vector3f pos, Vector3f vel);

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModel();

    static void loadModels();

    int getEntityType();

    void getHit(Vector3f* hitPos, Vector3f* hitDir, int weapon);

    void movingBlocksAreDone();

    void bounceOffOtherBalls(std::vector<Ball*>* balls);
};
#endif
