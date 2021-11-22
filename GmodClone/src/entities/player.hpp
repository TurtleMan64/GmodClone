#ifndef PLAYER_H
#define PLAYER_H

class TexturedModel;
class Triangle3D;
class Dummy;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Player : public Entity
{
public:
    Triangle3D* latestGroundTriangle = nullptr;
    Triangle3D* latestWallTriangle = nullptr;
    Vector3f groundNormal;
    Vector3f wallNormal;

    bool onGround = false;

    bool isTouchingWall = false;

    float stepTimer = 0.0f;
    float landSoundTimer = 0.0f;

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

    //float currentCollisionRadius = COLLISION_RADIUS;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float FORCE_PUSH_GROUND = 46.385f;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float FORCE_PUSH_AIR = 9.924f;

    const float FORCE_GRAVITY = 540.0f/36.7816091954f;

    const float DRAG_GROUND = 8.15f;

    const float DRAG_AIR = 1.0;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float JUMP_SPEED = 5.33f;

    //for stored jumps
    float timeSinceOnGround = 0.0f;
    Vector3f lastGroundNormal;
    const float AIR_JUMP_TOLERANCE = 0.02f;

    //const float WALL_JUMP_SPEED_VERTICAL = 4.0f;

    const float WALL_JUMP_SPEED_HORIZONTAL = 5.0f;

    // How far you can hit things from
    const float ARM_REACH = 1.9f;
    float swingArmTimer = 0.0f;
    const float SWING_ARM_COOLDOWN  = 0.4f;

    // > 0 means you cant use a ladder (colldown after jumping)
    float ladderTimer = 0.0f;
    const float LADDER_JUMP_COOLDOWN = 1.0f;
    bool isOnLadder = false;

    void updateCamera();

    void swingYourArm();

    float getPushValueGround(float deltaTime);

    float getPushValueAir(float deltaTime);

    float getJumpValue(float deltaTime);

    Entity* collideEntityImTouching = nullptr;
    Vector3f externalVel;
    Vector3f externalVelPrev;
    Vector3f lookDir;
    bool isCrouching = false;

    char weapon = 0;

    char health = 100;

    Dummy* weaponModel = nullptr;

    Player();

    void step();

    std::list<TexturedModel*>* getModels();

    void die();
};
#endif
