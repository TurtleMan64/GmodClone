#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

class Vao;
class Joint;
class Animation;

#include <glad/glad.h>

#include <vector>
#include <string>
#include <unordered_map>

#include "../toolbox/matrix.hpp"
#include "keyframe.hpp"

class AnimatedModel
{
public:
    // All of the position, texture, normal, skin weights, etc. that the model uses in the shader.
    Vao* model = nullptr;
    GLuint texture = GL_NONE;
    int jointCount = 0;

private:
    // The root joint of the model.
    Joint* rootJoint = nullptr;

public:
    AnimatedModel();

    AnimatedModel(Vao* model, GLuint texture, Joint* rootJoint, int jointCount);

    void deleteMe();

    // Calculates the poses of each bone for an animation at a given time.
    std::unordered_map<std::string, Matrix4f> calculateAnimationPose(Animation* animation, float time);

    // Calculates the poses of each bone by blending together two animations.
    std::unordered_map<std::string, Matrix4f> calculateAnimationPose(Animation* animation1, float time1, Animation* animation2, float time2, float blend);

    // Given a pose, applies the joint transforms and stores them in the given joint transforms vector.
    void calculateJointTransformsFromPose(std::vector<Matrix4f>* outJointTransforms, std::unordered_map<std::string, Matrix4f>* pose);

private:
    void addJointsToArray(Joint* headJoint, std::vector<Matrix4f>* jointMatrices);

    void calculateJointTransforms(std::vector<Matrix4f>* outJointTransforms);

    void applyPoseToJoints(std::unordered_map<std::string, Matrix4f>* currentPose, Joint* joint, Matrix4f* parentTransform);

    std::vector<Keyframe> getPreviousAndNextFrames(Animation* animation, float time);

    float calculateProgression(Keyframe* previousFrame, Keyframe* nextFrame, float time);

    std::unordered_map<std::string, Matrix4f> interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression);
};
#endif
