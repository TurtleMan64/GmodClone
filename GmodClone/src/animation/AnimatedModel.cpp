#include <glad/glad.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "animatedmodel.hpp"
#include "animation.hpp"
#include "joint.hpp"
#include "../openglObjects/vao.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../main/main.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../toolbox/split.hpp"
#include "../toolbox/matrix.hpp"

AnimatedModel::AnimatedModel()
{

}

AnimatedModel::AnimatedModel(Vao* model, GLuint texture, Joint* rootJoint, int jointCount)
{
    this->model = model;
    this->texture = texture;
    this->rootJoint = rootJoint;
    this->jointCount = jointCount;
}

void AnimatedModel::deleteMe()
{
    model->deleteMe();
    delete model; INCR_DEL("Vao");
    model = nullptr;

    rootJoint->deleteChildren();
    delete rootJoint; INCR_DEL("Joint");
    rootJoint = nullptr;

    Loader::deleteTexture(texture);
}

std::unordered_map<std::string, JointTransform> AnimatedModel::calculateAnimationPose(Animation* animation, float time)
{
    if (time > animation->length)
    {
        if (animation->timeWrappingType == 0) //looping
        {
            time = fmodf(time, animation->length);
        }
        else
        {
            time = animation->length - 0.00001f;
        }
    }

    std::vector<Keyframe> frames = getPreviousAndNextFrames(animation, time);
    float progression = calculateProgression(&frames[0], &frames[1], time);
    return interpolatePoses(&frames[0], &frames[1], progression);
}

std::unordered_map<std::string, JointTransform> AnimatedModel::calculateAnimationPose(Animation* animation1, float time1, Animation* animation2, float time2, float blend)
{
    if (time1 > animation1->length)
    {
        if (animation1->timeWrappingType == 0) //looping
        {
            time1 = fmodf(time1, animation1->length);
        }
        else
        {
            time1 = animation1->length - 0.00001f;
        }
    }

    if (time2 > animation2->length)
    {
        if (animation2->timeWrappingType == 0) //looping
        {
            time2 = fmodf(time2, animation2->length);
        }
        else
        {
            time2 = animation2->length - 0.00001f;
        }
    }

    blend = Maths::clamp(0.0f, 1.0f - blend, 0.999999f);

    std::vector<Keyframe> frames1 = getPreviousAndNextFrames(animation1, time1);
    float progression1 = calculateProgression(&frames1[0], &frames1[1], time1);
    std::unordered_map<std::string, JointTransform> pose1 = interpolatePoses(&frames1[0], &frames1[1], progression1);

    std::vector<Keyframe> frames2 = getPreviousAndNextFrames(animation2, time2);
    float progression2 = calculateProgression(&frames2[0], &frames2[1], time2);
    std::unordered_map<std::string, JointTransform> pose2 = interpolatePoses(&frames2[0], &frames2[1], progression2);

    std::unordered_map<std::string, JointTransform> finalPose;

    for (auto entry : pose1)
    {
        JointTransform previousTransform = pose1[entry.first];
        JointTransform nextTransform = pose2[entry.first];
        JointTransform currentTransform = JointTransform::interpolate(&previousTransform, &nextTransform, blend);
        finalPose[entry.first] = currentTransform;
    }

    return finalPose;
}

void AnimatedModel::calculateJointTransformsFromPose(std::vector<Matrix4f>* outJointTransforms, std::unordered_map<std::string, JointTransform>* pose)
{
    Matrix4f identity;
    applyPoseToJoints(pose, rootJoint, &identity);
    calculateJointTransforms(outJointTransforms);
}




//private helpers

void AnimatedModel::calculateJointTransforms(std::vector<Matrix4f>* outJointTransforms)
{
    addJointsToArray(rootJoint, outJointTransforms);
}

void AnimatedModel::addJointsToArray(Joint* headJoint, std::vector<Matrix4f>* jointMatrices)
{
    jointMatrices->at(headJoint->index) = headJoint->animatedTransform;
    for (Joint* childJoint : headJoint->children)
    {
        addJointsToArray(childJoint, jointMatrices);
    }
}

void AnimatedModel::applyPoseToJoints(std::unordered_map<std::string, JointTransform>* currentPose, Joint* joint, Matrix4f* parentTransform)
{
    JointTransform jointTransform = (*currentPose)[joint->name];

    bool isRootJoint = this->rootJoint == joint;

    Matrix4f currentLocalTransform;
    jointTransform.calculateLocalTransform(&currentLocalTransform, isRootJoint);

    Matrix4f currentTransform;
    parentTransform->multiply(&currentLocalTransform, &currentTransform);

    for (Joint* childJoint : joint->children)
    {
        applyPoseToJoints(currentPose, childJoint, &currentTransform);
    }
    currentTransform.multiply(&joint->inverseBindTransform, &currentTransform);

    joint->animatedTransform = currentTransform;
}

std::vector<Keyframe> AnimatedModel::getPreviousAndNextFrames(Animation* animation, float time)
{
    std::vector<Keyframe> allFrames = animation->keyframes;
    Keyframe previousFrame = allFrames[0];
    Keyframe nextFrame = allFrames[0];
    for (int i = 1; i < (int)allFrames.size(); i++)
    {
        nextFrame = allFrames[i];
        if (nextFrame.timeStamp > time)
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

float AnimatedModel::calculateProgression(Keyframe* previousFrame, Keyframe* nextFrame, float time)
{
    float totalTime = nextFrame->timeStamp - previousFrame->timeStamp;
    float currentTime = time - previousFrame->timeStamp;
    return currentTime / totalTime;
}

//std::unordered_map<std::string, Matrix4f> AnimatedModel::interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression)
//{
//    std::unordered_map<std::string, Matrix4f> currentPose;
//    for (auto entry : previousFrame->pose)
//    {
//        JointTransform previousTransform = previousFrame->pose[entry.first];
//        JointTransform nextTransform = nextFrame->pose[entry.first];
//        JointTransform currentTransform = JointTransform::interpolate(&previousTransform, &nextTransform, progression);
//        Matrix4f localTransform;
//        currentTransform.calculateLocalTransform(&localTransform);
//        currentPose[entry.first] = localTransform;
//    }
//    return currentPose;
//}

std::unordered_map<std::string, JointTransform> AnimatedModel::interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression)
{
    std::unordered_map<std::string, JointTransform> currentPose;
    for (auto entry : previousFrame->pose)
    {
        JointTransform previousTransform = previousFrame->pose[entry.first];
        JointTransform nextTransform = nextFrame->pose[entry.first];
        JointTransform currentTransform = JointTransform::interpolate(&previousTransform, &nextTransform, progression);
        currentPose[entry.first] = currentTransform;
    }
    return currentPose;
}

