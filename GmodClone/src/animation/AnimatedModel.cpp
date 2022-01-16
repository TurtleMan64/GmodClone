#include <glad/glad.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "animatedmodel.hpp"
#include "animation.hpp"
#include "animator.hpp"
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
    //texture also needs to delete
    Loader::deleteTexture(texture);
}

std::unordered_map<std::string, Matrix4f> AnimatedModel::calculateAnimationPose(Animation* animation, float time)
{
    if (time > animation->length)
    {
        time = fmodf(time, animation->length);
    }

    std::vector<Keyframe> frames = getPreviousAndNextFrames(animation, time);
    float progression = calculateProgression(&frames[0], &frames[1], time);
    return interpolatePoses(&frames[0], &frames[1], progression);
}

std::unordered_map<std::string, Matrix4f> AnimatedModel::calculateAnimationPose(Animation* animation1, float time1, Animation* animation2, float time2, float blend)
{
    //if (time > animation->length)
    //{
    //    time = fmodf(time, animation->length);
    //}
    //
    //std::vector<Keyframe> frames = getPreviousAndNextFrames(animation, time);
    //float progression = calculateProgression(&frames[0], &frames[1], time);
    //return interpolatePoses(&frames[0], &frames[1], progression);
}

void AnimatedModel::calculateJointTransformsFromPose(std::vector<Matrix4f>* outJointTransforms, std::unordered_map<std::string, Matrix4f>* pose)
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

void AnimatedModel::applyPoseToJoints(std::unordered_map<std::string, Matrix4f>* currentPose, Joint* joint, Matrix4f* parentTransform)
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

std::unordered_map<std::string, Matrix4f> AnimatedModel::interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression)
{
    std::unordered_map<std::string, Matrix4f> currentPose;
    for (auto entry : previousFrame->pose)
    {
        JointTransform previousTransform = previousFrame->pose[entry.first];
        JointTransform nextTransform = nextFrame->pose[entry.first];
        JointTransform currentTransform = JointTransform::interpolate(&previousTransform, &nextTransform, progression);
        Matrix4f localTransform;
        currentTransform.calculateLocalTransform(&localTransform);
        currentPose[entry.first] = localTransform;
    }
    return currentPose;
}
