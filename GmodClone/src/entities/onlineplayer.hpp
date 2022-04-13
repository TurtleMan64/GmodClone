#ifndef ONLINEPLAYER_H
#define ONLINEPLAYER_H

class TexturedModel;
class GUIText;
class Dummy;
class AnimatedModel;
class Animation;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class OnlinePlayer : public Entity
{
private:
    GUIText* nametag = nullptr;

    Animation* getAnimation(char index);
    float getAnimationTimer(char index);

public:
    bool inputAction3 = false;
    bool inputAction4 = false;
    float inputX = 0.0f;
    float inputY = 0.0f;
    float inputX2 = 0.0f;
    float inputY2 = 0.0f;

    int pingMs = 0;

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

    char weapon = WEAPON_FIST;

    char health = 100;

    Dummy* entityWeapon = nullptr;
    Dummy* entityHookshot = nullptr;
    Dummy* entityHookshotTip = nullptr;
    Dummy* entityHookshotHandle = nullptr;
    Dummy* entityHookshotChain = nullptr;

    bool isOnRope = false;
    Vector3f ropeAnchor;
    float ropeLength = 1.0f;

    float inZoneTime = 0.0f;

    float animTimerStand   = 0.0f;
    float animTimerWalk    = 0.0f;
    float animTimerRun     = 0.0f;
    float animTimerCrouch  = 0.0f;
    float animTimerSlide   = 0.0f;
    float animTimerJump    = 0.0f;
    float animTimerFall    = 0.0f;
    float animTimerClimb   = 0.0f;
    float animTimerCrawl   = 0.0f;
    float animTimerSwing   = 0.0f;
    char  animType         = 0;
    char  animTypePrevious = 0;
    float animBlend        = 0.0f;

    OnlinePlayer(std::string name, float x, float y, float z);
    ~OnlinePlayer();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    Model* getModels();

    AnimatedModel* getAnimatedModel();

    static void loadModels();

    int getEntityType();

    void getHit(Vector3f* hitPos, Vector3f* hitDir, int weapon);
};
#endif
