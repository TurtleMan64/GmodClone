#include <list>
#include <cstring>

#include "onlineplayer.hpp"
#include "../main/main.hpp"
#include "../loader/objloader.hpp"
#include "../toolbox/maths.hpp"

#include "../toolbox/vector.hpp"
#include "player.hpp"
#include "../collision/collisionchecker.hpp"
#include "../collision/triangle3d.hpp"
#include "../toolbox/input.hpp"
#include "../main/main.hpp"
#include "camera.hpp"
#include "ball.hpp"
#include "ladder.hpp"
#include "dummy.hpp"
#include "../fontMeshCreator/guitext.hpp"
#include "../audio/audioplayer.hpp"
#include "../animation/animation.hpp"
#include "../animation/animatedmodel.hpp"
#include "../animation/animatedmodelloader.hpp"
#include "../animation/animationloader.hpp"
#include "../animation/jointtransform.hpp"
#include "../toolbox/quaternion.hpp"

extern float dt;

OnlinePlayer::OnlinePlayer(std::string name, float x, float y, float z)
{
    this->name = name;
    visible = true;
    position.set(x, y, z);
    vel.set(0.000001f, 0, 0);
    lookDir.set(0, 0, -1);
    groundNormal.set(0, 1, 0);
    wallNormal.set(1, 0, 0);
    lastGroundNormal.set(0, 1, 0);
    updateTransformationMatrix();

    for (int i = 0; i < Player::modelShrek->jointCount; i++)
    {
        Matrix4f mat;
        jointTransforms.push_back(mat);
    }

    entityWeapon         = new Dummy(&Player::modelGun);            INCR_NEW("Dummy");
    entityHookshot       = new Dummy(&Player::modelHookshot);       INCR_NEW("Dummy");
    entityHookshotTip    = new Dummy(&Player::modelHookshotTip);    INCR_NEW("Dummy");
    entityHookshotHandle = new Dummy(&Player::modelHookshotHandle); INCR_NEW("Dummy");
    entityHookshotChain  = new Dummy(&Player::modelHookshotChain);  INCR_NEW("Dummy");

    entityWeapon        ->visible = false;
    entityHookshot      ->visible = false;
    entityHookshotTip   ->visible = false;
    entityHookshotHandle->visible = false;
    entityHookshotChain ->visible = false;

    entitiesToRender.push_back(entityWeapon);
    entitiesToRender.push_back(entityHookshot);
    entitiesToRender.push_back(entityHookshotTip);
    entitiesToRender.push_back(entityHookshotHandle);
    entitiesToRender.push_back(entityHookshotChain);
}

OnlinePlayer::~OnlinePlayer()
{
    if (nametag != nullptr)
    {
        Global::gameOnlinePlayersSharedMutex.lock();
        Global::gameOnlinePlayerNametagsToDelete.push_back(nametag);
        Global::gameOnlinePlayersSharedMutex.unlock();

        nametag = nullptr;
    }

    if (entityWeapon         != nullptr) { delete entityWeapon;         INCR_DEL("Dummy"); entityWeapon         = nullptr; }
    if (entityHookshot       != nullptr) { delete entityHookshot;       INCR_DEL("Dummy"); entityHookshot       = nullptr; }
    if (entityHookshotTip    != nullptr) { delete entityHookshotTip;    INCR_DEL("Dummy"); entityHookshotTip    = nullptr; }
    if (entityHookshotHandle != nullptr) { delete entityHookshotHandle; INCR_DEL("Dummy"); entityHookshotHandle = nullptr; }
    if (entityHookshotChain  != nullptr) { delete entityHookshotChain;  INCR_DEL("Dummy"); entityHookshotChain  = nullptr; }
}

void OnlinePlayer::step()
{
    Vector3f yAxis(0, -1, 0);

    updateCamera();

    position = position + vel.scaleCopy(dt) + externalVel.scaleCopy(dt);

    visible = true;

    if (health <= 0)
    {
        visible = false;
    }

    Vector3f nametagPos = position;
    nametagPos.y += eyeHeightSmooth + 0.25f;

    Vector3f diff = Global::gameCamera->eye - nametagPos;

    Vector2f screenPos = Maths::calcScreenCoordsOfWorldPoint(&nametagPos);

    if (nametag == nullptr)
    {
        nametag = new GUIText(name, 0.1f, Global::fontConsolas, -0.5f, -0.5f, 4, true); INCR_NEW("GUIText");
    }

    if (health <= 0 || diff.lengthSquared() > 20*20)
    {
        nametag->visible = false;
    }
    else
    {
        //todo: enable this once checkCollision has been optimized for line segments

        CollisionResult result = CollisionChecker::checkCollision(&Global::gameCamera->eye, &nametagPos);
        nametag->visible = !result.hit;

        //nametag->visible = true;
    }

    nametag->fontSize = 0.15f / diff.length();
    nametag->position = screenPos;



    // Animate the player
    float animSpd = vel.length();

    char animTypeNew = 0;

    if (isOnLadder)
    {
        animTypeNew = 7;
        animTimerClimb += 3*animSpd*dt;
    }
    else if (isCrouching && timeSinceOnGround <= 0.04f)
    {
        if (animSpd > 4.0f)
        {
            animTypeNew = 8;
            animTimerCrawl += 0.45f*animSpd*dt;
        }
        else
        {
            animTypeNew = 3;
            animTimerCrouch += 0.45f*animSpd*dt;
        }
    }
    else if (slideTimer > 0.0f)
    {
        animTypeNew = 4;
        animTimerSlide += 1.5f*dt;
    }
    else if (timeSinceOnGround <= 0.04f) //on ground
    {
        if (animSpd < 0.2f) //stand
        {
            animTypeNew = 0;
            animTimerStand += dt;
        }
        else if (animSpd < 6.0f) //walk
        {
            animTypeNew = 1;
            animTimerRun += 0.18f*animSpd*dt;
        }
        else //run
        {
            animTypeNew = 2;
            animTimerRun += 0.18f*animSpd*dt;
        }
    }
    else //in air
    {
        animTypeNew = 6;
        animTimerFall += dt;
    }

    if (animTypeNew != 6)
    {
        animTimerFall = 0.0f;
    }

    if (animType != animTypeNew)
    {
        animTypePrevious = animType;
        animType = animTypeNew;
        animBlend = 0.0f;
    }
    else
    {
        animBlend += 10*dt;
    }


    std::unordered_map<std::string, JointTransform> pose;

    if (animBlend <= 1.0f)
    {
        pose = Player::modelShrek->calculateAnimationPose(
            getAnimation(animType),
            getAnimationTimer(animType),
            getAnimation(animTypePrevious),
            getAnimationTimer(animTypePrevious),
            animBlend);
    }
    else
    {
        pose = Player::modelShrek->calculateAnimationPose(
            getAnimation(animType),
            getAnimationTimer(animType));
    }

    if (animType == 7)//ladder
    {
        Vector3f velFlat = lookDir;
        velFlat.y = 0;
        if (fabsf(velFlat.x) > fabsf(velFlat.z))
        {
            velFlat.z = 0;
        }
        else
        {
            velFlat.x = 0;
        }

        velFlat.inv();

        velFlat.normalize();

        Vector3f newPos = position + velFlat.scaleCopy(-0.25f);

        pose["Hips"].lookAtAndTranslate(velFlat, newPos);
    }
    else
    {
        Vector3f velFlat = vel;
        velFlat.y = 0;
        if (velFlat.lengthSquared() > 0.5f*0.5f)
        {
            if (slideTimer > 0.0f || animType == 8)
            {
                pose["Hips"].lookAtAndTranslate(vel, position);
            }
            else
            {
                pose["Hips"].lookAtAndTranslate(velFlat, position);
            }
        }
        else
        {
            Vector3f lookFlat = lookDir;
            lookFlat.y = 0;
            pose["Hips"].lookAtAndTranslate(lookFlat, position);
        }
    }

    float directionHead = atan2f(lookDir.y, sqrtf(lookDir.x*lookDir.x + lookDir.z*lookDir.z));
    Quaternion myRotationPitch = Quaternion::fromEulerAngles(0, 0, directionHead);
    pose["Head"].rotation = Quaternion::multiply(pose["Head"].rotation, myRotationPitch);

    Player::modelShrek->calculateJointTransformsFromPose(&jointTransforms, &pose);

    // Animating our weapons / hookshot
    if (isOnRope)
    {
        entityHookshot      ->visible = false;
        entityHookshotHandle->visible = true;
        entityHookshotTip   ->visible = true;
        entityHookshotChain ->visible = true;

        Vector4f handPos = Vector4f(0.790331f, 1.40341f, -0.002674f, 1.0f);
        handPos = jointTransforms[11].transform(&handPos);
        entityHookshotHandle->position.set(handPos.x, handPos.y, handPos.z);

        Matrix4f rotMat = Quaternion::fromMatrix(&jointTransforms[11]).toRotationMatrix();

        entityHookshotHandle->transformationMatrix.setIdentity();
        entityHookshotHandle->transformationMatrix.translate(&entityHookshotHandle->position);
        entityHookshotHandle->transformationMatrix.multiply(&rotMat, &entityHookshotHandle->transformationMatrix);

        Vector4f hookPos = Vector4f(0.790331f + 0.2f, 1.40341f, -0.002674f, 1.0f);
        hookPos = jointTransforms[11].transform(&hookPos);
        entityHookshotChain->position.set(hookPos.x, hookPos.y, hookPos.z);

        Vector3f toAnchor = ropeAnchor - entityHookshotChain->position;
        Maths::sphereAnglesFromPosition(&toAnchor, &entityHookshotChain->rotY, &entityHookshotChain->rotZ);
        entityHookshotChain->updateTransformationMatrix(toAnchor.length() - 0.2f, 1.0f, 1.0f);

        entityHookshotTip->position = ropeAnchor;
        entityHookshotTip->rotY = entityHookshotChain->rotY;
        entityHookshotTip->rotZ = entityHookshotChain->rotZ;
        entityHookshotTip->updateTransformationMatrix();
    }
    else
    {
        entityHookshot      ->visible = true;
        entityHookshotHandle->visible = false;
        entityHookshotTip   ->visible = false;
        entityHookshotChain ->visible = false;

        Vector4f handPos = Vector4f(0.790331f, 1.40341f, -0.002674f, 1.0f);
        handPos = jointTransforms[11].transform(&handPos);
        entityHookshot->position.set(handPos.x, handPos.y, handPos.z);

        Matrix4f rotMat = Quaternion::fromMatrix(&jointTransforms[11]).toRotationMatrix();

        entityHookshot->transformationMatrix.setIdentity();
        entityHookshot->transformationMatrix.translate(&entityHookshot->position);
        entityHookshot->transformationMatrix.multiply(&rotMat, &entityHookshot->transformationMatrix);
    }

    Vector3f xAx(1, 0, 0);
    Vector3f yAx(0, 1, 0);
    Vector3f zAx(0, 0, 1);

    switch (weapon)
    {
        case WEAPON_FIST:
        {
            entityWeapon->visible = false;
            break;
        }

        case WEAPON_BAT:
        {
            entityWeapon->visible = true;
            entityWeapon->setModels(&Player::modelBat);

            Vector4f handPos = Vector4f(-0.790331f, 1.40341f, -0.002674f, 1.0f);
            handPos = jointTransforms[2].transform(&handPos);
            entityWeapon->position.set(handPos.x, handPos.y, handPos.z);

            Matrix4f rotMat = Quaternion::fromMatrix(&jointTransforms[2]).toRotationMatrix();

            Matrix4f rotOff = Quaternion::fromEulerAngles(-Maths::PI/2, 0, 0).toRotationMatrix();

            entityWeapon->transformationMatrix.setIdentity();
            entityWeapon->transformationMatrix.translate(&entityWeapon->position);
            entityWeapon->transformationMatrix.multiply(&rotMat, &entityWeapon->transformationMatrix);
            entityWeapon->transformationMatrix.multiply(&rotOff, &entityWeapon->transformationMatrix);
            break;
        }

        case WEAPON_GUN:
        {
            //entityWeapon->visible = true;
            //entityWeapon->setModels(&Player::modelGun);

            entityWeapon->visible = false;
            break;
        }

        default: break;
    }
}

std::vector<Entity*>* OnlinePlayer::getEntitiesToRender()
{
    if (health > 0)
    {
        return &entitiesToRender;
    }

    return nullptr;
}

Model* OnlinePlayer::getModel()
{
    return nullptr;
}

AnimatedModel* OnlinePlayer::getAnimatedModel()
{
    if (health > 0)
    {
        return Player::modelShrek;
    }

    return nullptr;
}

int OnlinePlayer::getEntityType()
{
    return ENTITY_ONLINE_PLAYER;
}

void OnlinePlayer::getHit(Vector3f* hitPos, Vector3f* hitDir, int wpn)
{
    Message msg;
    msg.buf[0] = 4;

    int nameLen = (int)name.size();
    memcpy(&msg.buf[1], &nameLen, 4);
    memcpy(&msg.buf[5], name.c_str(), nameLen);

    memcpy(&msg.buf[ 5 + nameLen], &hitPos->x, 4);
    memcpy(&msg.buf[ 9 + nameLen], &hitPos->y, 4);
    memcpy(&msg.buf[13 + nameLen], &hitPos->z, 4);
    memcpy(&msg.buf[17 + nameLen], &hitDir->x, 4);
    memcpy(&msg.buf[21 + nameLen], &hitDir->y, 4);
    memcpy(&msg.buf[25 + nameLen], &hitDir->z, 4);

    char dub = (char)wpn;
    memcpy(&msg.buf[29 + nameLen], &dub, 1);

    msg.length = 1 + 4 + nameLen + 24 + 1;

    Global::sendMessageToServer(msg);

    switch (wpn)
    {
        case WEAPON_FIST: AudioPlayer::play(75, nullptr); break;
        case WEAPON_BAT:  AudioPlayer::play(74, nullptr); break;
        case WEAPON_GUN:  AudioPlayer::play(74, nullptr); break;
        default: break;
    }
}

float OnlinePlayer::getPushValueGround(float deltaTime)
{
    double fps = 1.0/deltaTime;

    double val;

    if (inputAction3) //Running
    {
        val = 66.56795 + (17926930.0 - 66.56795)/(1.0 + pow(fps/0.00004730095, 1.080826));
    }
    else //Walking
    {
        val = 44.35799 + (17431400.0 - 44.35799)/(1.0 + pow(fps/0.00002812362, 1.065912));
    }

    if (isCrouching)
    {
        val = val - 25.0f;
    }

    return (float)val;
}

float OnlinePlayer::getPushValueAir(float deltaTime)
{
    double fps = 1.0/deltaTime;
    return (float)(9.917682 + (14.17576 - 9.917682)/(1.0 + pow(fps/1.386981, 1.042293)));
}

void OnlinePlayer::updateCamera()
{
    Vector3f yAxis(0, 1, 0);

    inputX2 = 0;
    inputY2 = 0;

    lookDir = Maths::rotatePoint(&lookDir, &yAxis, -inputX2);
    lookDir.normalize();

    Vector3f perpen = lookDir.cross(&yAxis);
    perpen.normalize();

    const float maxViewAng = Maths::toRadians(0.1f);

    if (inputY2 > 0) //mouse is going down
    {
        Vector3f down(0, -1, 0);
        float angBetweenCamAndDown = Maths::angleBetweenVectors(&lookDir, &down);
        float angToRotate = inputY2;
        if (angBetweenCamAndDown - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -inputY2);
        }
        else
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -(angBetweenCamAndDown - maxViewAng));
        }
    }
    else
    {
        Vector3f up(0, 1, 0);
        float angBetweenCamAndUp = Maths::angleBetweenVectors(&lookDir, &up);
        float angToRotate = -inputY2;
        if (angBetweenCamAndUp - angToRotate > maxViewAng)
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, -inputY2);
        }
        else
        {
            lookDir = Maths::rotatePoint(&lookDir, &perpen, (angBetweenCamAndUp - maxViewAng));
        }
    }
    lookDir.normalize();

    if (isCrouching || slideTimer > 0.0f)
    {
        eyeHeightSmooth  = Maths::approach(eyeHeightSmooth, HUMAN_HEIGHT/2 - 0.07f, 12.0f, dt);
    }
    else
    {
        eyeHeightSmooth  = Maths::approach(eyeHeightSmooth, HUMAN_HEIGHT - 0.07f, 12.0f, dt);
    }
}

void OnlinePlayer::loadModels()
{
    //if (OnlinePlayer::modelhrek == nullptr)
    //{
    //    OnlinePlayer::modelhrek = AnimatedModelLoader::loadAnimatedModel("res/Models/Human/", "ShrekFinalMeshFrankenstein.mesh");
    //
    //    OnlinePlayer::animationStand  = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Breathing Idle.anim");
    //    OnlinePlayer::animationWalk   = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Slow Run.anim");
    //    OnlinePlayer::animationRun    = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Fast Run.anim");
    //    OnlinePlayer::animationCrouch = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Crouched Walking.anim");
    //    OnlinePlayer::animationCrawl  = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Running Crawl.anim");
    //    OnlinePlayer::animationSlide  = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Running Slide 2.anim");
    //    OnlinePlayer::animationJump   = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Jumping Up.anim");
    //    OnlinePlayer::animationFall   = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/Falling Idle.anim");
    //    OnlinePlayer::animationClimb  = AnimationLoader::loadAnimation("res/Models/Human/Original Mixamo/BlenderOutput/Climbing Ladder.anim");
    //}
}

Animation* OnlinePlayer::getAnimation(char index)
{
    switch (index)
    {
        case 0:  return Player::animationStand;
        case 1:  return Player::animationWalk;
        case 2:  return Player::animationRun;
        case 3:  return Player::animationCrouch;
        case 4:  return Player::animationSlide;
        case 5:  return Player::animationJump;
        case 6:  return Player::animationFall;
        case 7:  return Player::animationClimb;
        case 8:  return Player::animationCrawl;
        default: return Player::animationStand;
    }
}

float OnlinePlayer::getAnimationTimer(char index)
{
    switch (index)
    {
        case 0:  return animTimerStand;
        case 1:  return animTimerRun; //use the same timer for walk and run
        case 2:  return animTimerRun;
        case 3:  return animTimerCrouch;
        case 4:  return animTimerSlide;
        case 5:  return animTimerJump;
        case 6:  return animTimerFall;
        case 7:  return animTimerClimb;
        case 8:  return animTimerCrawl;
        default: return animTimerStand;
    }
}
