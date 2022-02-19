#ifndef PLAYER_H
#define PLAYER_H

class TexturedModel;
class Triangle3D;
class Dummy;
class AnimatedModel;
class Animation;

#include <list>
#include "entity.hpp"
#include "../toolbox/vector.hpp"

class Player : public Entity
{
private:
    static AnimatedModel* modelShrek;

    static Animation* animationStand;
    static Animation* animationWalk;
    static Animation* animationRun;
    static Animation* animationCrouch;
    static Animation* animationCrawl;
    static Animation* animationSlide;
    static Animation* animationJump;
    static Animation* animationFall;
    static Animation* animationClimb;
    static Animation* animationSwing;

    void animateMe();
    Animation* getAnimation(char index);
    float getAnimationTimer(char index);

public:
    Triangle3D* latestGroundTriangle = nullptr;
    Triangle3D* latestWallTriangle = nullptr;
    Vector3f groundNormal;
    Vector3f wallNormal;

    bool onGround = false;

    bool isTouchingWall = false;

    float stepTimer = 0.0f;
    float landSoundTimer = 0.0f;

    static constexpr float SLIDE_SPEED_REQUIRED = 4.0f;
    static constexpr float SLIDE_TIMER_DURATION = 0.666f;
    static constexpr float SLIDE_TIMER_COOLDOWN = 0.3f;
    static constexpr float SLIDE_SPEED_ADDITION = 4.0f;
    float slideTimer = 0.0f;
    float storedSlideSpeed = 0.0f;

    static constexpr float WALL_JUMP_TIMER_MAX = 0.1f;
    float wallJumpTimer = 0.0f;
    Vector3f storedWallNormal;

    static constexpr float HUMAN_HEIGHT = 1.74f; //1.74 meters

    float eyeHeightSmooth = HUMAN_HEIGHT;

    static constexpr float COLLISION_RADIUS = HUMAN_HEIGHT/4; //collision is 2 spheres

    //float currentCollisionRadius = COLLISION_RADIUS;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float FORCE_PUSH_GROUND = 46.385f;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float FORCE_PUSH_AIR = 9.924f;

    static constexpr float FORCE_GRAVITY = 540.0f/36.7816091954f;

    static constexpr float DRAG_GROUND = 8.15f;

    static constexpr float DRAG_AIR = 1.0;

    //replaced with a function that calculates this, since its slightly different based on fps
    //const float JUMP_SPEED = 5.33f;

    //for stored jumps
    float timeSinceOnGround = 0.0f;
    Vector3f lastGroundNormal;
    static constexpr float AIR_JUMP_TOLERANCE = 0.02f;

    //const float WALL_JUMP_SPEED_VERTICAL = 4.0f;

    static constexpr float WALL_JUMP_SPEED_HORIZONTAL = 5.0f;

    // How far you can hit things from
    static constexpr float WEAPON_REACH_FIST = 1.9f;
    static constexpr float WEAPON_REACH_BAT  = 2.7f;
    static constexpr float WEAPON_REACH_GUN  = 100.0f;

    float useWeaponTimer = 0.0f;
    static constexpr float WEAPON_COOLDOWN_FIST = 0.2f;
    static constexpr float WEAPON_COOLDOWN_BAT  = 0.4f;
    static constexpr float WEAPON_COOLDOWN_GUN  = 0.1f;

    // > 0 means you cant use a ladder (colldown after jumping)
    float ladderTimer = 0.0f;
    static constexpr float LADDER_JUMP_COOLDOWN = 1.0f;
    bool isOnLadder = false;

    // How much time you have been in king of the hill for.
    float inZoneTime = 0.0f;

    void updateCamera();

    void useWeapon();

    float getPushValueGround(float deltaTime);

    float getPushValueAir(float deltaTime);

    float getJumpValue(float deltaTime);

    int getSoundEffectFootstep(int soundType);

    int getSoundEffectImpact(int soundType);

    Entity* collideEntityImTouching = nullptr;
    Vector3f externalVel;
    Vector3f externalVelPrev;
    Vector3f lookDir;
    bool isCrouching = false;

    char weapon = 0;

    static constexpr float FALL_DAMAGE_SPEED_START = 16.0f;
    static constexpr float FALL_DAMAGE_SPEED_FATAL = 36.0f;

    char health = 100;

    Dummy* weaponModel = nullptr;

    // Animation variables
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

    Player();
    ~Player();

    void reset();

    void step();

    std::vector<Entity*>* getEntitiesToRender();

    std::list<TexturedModel*>* getModels();

    AnimatedModel* getAnimatedModel();

    void die();
};
#endif
