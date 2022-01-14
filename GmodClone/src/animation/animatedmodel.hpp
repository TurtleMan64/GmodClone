#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

class Vao;
class Joint;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "animator.hpp"
#include "animation.hpp"

class AnimatedModel
{
public:
    //skin
    Vao* model = nullptr;
    GLuint texture = GL_NONE;

    //skeelton
    Joint* rootJoint = nullptr;
    int jointCount = 0;

    Animator* animator = nullptr;

    AnimatedModel();

    AnimatedModel(Vao* model, GLuint texture, Joint* rootJoint, int jointCount);

    void deleteMe();

    //void doAnimation(Animation* animation, float time);

    void update(Animation* animation, float time);

    std::vector<Matrix4f> calculateJointTransforms();

    void addJointsToArray(Joint* headJoint, std::vector<Matrix4f>* jointMatrices);
};
#endif
