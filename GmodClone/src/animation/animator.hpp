#ifndef ANIMATOR_H
#define ANIMATOR_H

class AnimatedModel;
class Animation;
class Joint;

#include <vector>
#include <unordered_map>
#include <string>

#include "animator.hpp"
#include "keyframe.hpp"
#include "../toolbox/matrix.hpp"

class Animator
{
public:
    AnimatedModel* entity = nullptr;

    Animation* currentAnimation = nullptr;
    float animationTime = 0.0f;

    Animator(AnimatedModel* entity);

    void doAnimation(Animation* animation);

    void update();

    void increaseAnimationTime();

    std::unordered_map<std::string, Matrix4f> calculateCurrentAnimationPose();

    void applyPoseToJoints(std::unordered_map<std::string, Matrix4f>* currentPose, Joint* joint, Matrix4f* parentTransform);

    std::vector<Keyframe> getPreviousAndNextFrames();

    float calculateProgression(Keyframe* previousFrame, Keyframe* nextFrame);

    std::unordered_map<std::string, Matrix4f> interpolatePoses(Keyframe* previousFrame, Keyframe* nextFrame, float progression);
};
#endif
