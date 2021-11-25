#ifndef ONLINEPLAYER_H
#define ONLINEPLAYER_H

class TexturedModel;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"
#include "dummy.hpp"

class OnlinePlayer : public Entity
{
private:
    static std::list<TexturedModel*> modelsHead;
    static std::list<TexturedModel*> modelsFall;
    static std::list<TexturedModel*> modelsJump;
    static std::list<TexturedModel*> modelsSlide;
    static std::list<TexturedModel*> modelsSquat;
    static std::list<TexturedModel*> modelsStand;

    Dummy* head = nullptr;

public:
    bool inputAction3 = false;
    bool inputAction4 = false;
    float inputX = 0.0f;
    float inputY = 0.0f;
    float inputX2 = 0.0f;
    float inputY2 = 0.0f;

    Vector3f groundNormal;
    Vector3f wallNormal;

    bool onGround = false;

    bool isTouchingWall = false;

    const float SLIDE_SPEED_REQUIRED = 4.0f;
    const float SLIDE_TIMER_DURATION = 0.666f;
    const float SLIDE_TIMER_COOLDOWN = 0.3f;
    const float SLIDE_SPEED_ADDITION = 4.0f;
    float slideTimer = 0.0f;
    float storedSlideSpeed = 0.0f;

    const float WALL_JUMP_TIMER_MAX = 0.1f;
    float wallJumpTimer = 0.0f;
    Vector3f storedWallNormal;

    const float HUMAN_HEIGHT = 1.74f; //1.74 meters

    float eyeHeightSmooth = HUMAN_HEIGHT;

    const float COLLISION_RADIUS = HUMAN_HEIGHT/4; //collision is 2 spheres 

    const float FORCE_GRAVITY = 540.0f/36.7816091954f;

    const float DRAG_GROUND = 8.15f;

    const float DRAG_AIR = 1.0;

    //for stored jumps
    float timeSinceOnGround = 0.0f;
    Vector3f lastGroundNormal;
    const float AIR_JUMP_TOLERANCE = 0.02f;

    const float WALL_JUMP_SPEED_HORIZONTAL = 5.0f;

    // > 0 means you cant use a ladder (colldown after jumping)
    float ladderTimer = 0.0f;
    const float LADDER_JUMP_COOLDOWN = 1.0f;
    bool isOnLadder = false;

    void updateCamera();

    float getPushValueGround(float deltaTime);

    float getPushValueAir(float deltaTime);

    Entity* collideEntityImTouching = nullptr;
    Vector3f externalVel;
    Vector3f lookDir;
    bool isCrouching = false;

    char weapon = 0;

    char health = 100;

    OnlinePlayer(std::string name, float x, float y, float z);
    ~OnlinePlayer();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    static void loadModels();

    int getEntityType();

    void getHit(Vector3f* hitPos, Vector3f* hitDir, int weapon);
};
#endif
