#include <vector>

#include "animator.hpp"
#include "animatedmodel.hpp"
#include "keyframe.hpp"
#include "joint.hpp"
#include "../toolbox/matrix.hpp"

Animator::Animator(AnimatedModel* entity)
{
    this->entity = entity;
}

void Animator::doAnimation(Animation* animation)
{
    this->animationTime = 0.0f;
    this->currentAnimation = animation;
}

void Animator::update()
{
    if (currentAnimation == nullptr)
    {
        return;
    }

    increaseAnimationTime();

    std::unordered_map<std::string, Matrix4f> currentPose = calculateCurrentAnimationPose();
    Matrix4f identity;
    applyPoseToJoints(&currentPose, entity->rootJoint, &identity);
}

extern float dt;

void Animator::increaseAnimationTime()
{
    animationTime += dt;
    if (animationTime > currentAnimation->length)
    {
        animationTime = fmodf(animationTime, currentAnimation->length);
    }
}

std::unordered_map<std::string, Matrix4f> Animator::calculateCurrentAnimationPose()
{
    std::vector<Keyframe> frames = getPreviousAndNextFrames();
    float progression = calculateProgression(&frames[0], &frames[1]);
    return interpolatePoses(&frames[0], &frames[1], progression);
}

void Animator::applyPoseToJoints(std::unordered_map<std::string, Matrix4f>* currentPose, Joint* joint, Matrix4f* parentTransform)
{
    Matrix4f currentLocalTransform = (*currentPose)[joint->name];
    Matrix4f currentTransform;
    parentTransform->multiply(&currentLocalTransform, &currentTransform);
    
    for (Joint* childJoint : joint->children)
    {
        applyPoseToJoints(currentPose, childJoint, &currentTransform);
    }
    currentTransform.multiply(&joint->inverseBindTransform, &currentTransform);

    joint->animatedTransform = currentTransform;
}

std::vector<Keyframe> Animator::getPreviousAndNextFrames()
{
    std::vector<Keyframe> allFrames = currentAnimation->keyframes;
    Keyframe previousFrame = allFrames[0];
    Keyframe nextFrame = allFrames[0];
    for (int i = 1; i < (int)allFrames.size(); i++)
    {
        nextFrame = allFrames[i];
        if (nextFrame.timeStamp > animationTime)
        {
            break;
        }
        previousFrame = allFrames[i];
    }

    std::vector<Keyframe> fr;
    fr.push_back(previousFrame);
    fr.push_back(nextFrame);
    return fr;
}

float Animator::calculateProgression(Keyframe* previousFrame, Keyframe* nextFrame)
{
    float totalTime = nextFrame->timeStamp - previousFrame->timeStamp;
    float currentTime = animationTime - previousFrame->timeStamp;
    return currentTime / totalTime;
}

std::unordered_map<std::string, Matrix4f> Animator::interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression)
{
    std::unordered_map<std::string, Matrix4f> currentPose;
    for (auto entry : previousFrame->pose)
    {
        entry.first;
        JointTransform previousTransform = previousFrame->pose[entry.first];
        JointTransform nextTransform = nextFrame->pose[entry.first];
        JointTransform currentTransform = JointTransform::interpolate(&previousTransform, &nextTransform, progression);
        Matrix4f localTransform;
        currentTransform.calculateLocalTransform(&localTransform);
        currentPose[entry.first] = localTransform;
    }
    return currentPose;
}
