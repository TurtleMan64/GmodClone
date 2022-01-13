#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    this->animator = new Animator(this); INCR_NEW("Animator");
    Matrix4f identityMat;
    rootJoint->calcInverseBindTransform(&identityMat);
}

void AnimatedModel::deleteMe()
{
    delete animator; INCR_DEL("Animator");
    animator = nullptr;
    model->deleteMe();
    //texture also needs to delete
    Loader::deleteTexture(texture);
}

void AnimatedModel::doAnimation(Animation* animation)
{
    animator->doAnimation(animation);
}

void AnimatedModel::update()
{
    animator->update();
}

std::vector<Matrix4f> AnimatedModel::calculateJointTransforms()
{
    std::vector<Matrix4f> jointMatrices;
    for (int i = 0; i < jointCount; i++)
    {
        Matrix4f mat;
        jointMatrices.push_back(mat);
    }
    addJointsToArray(rootJoint, &jointMatrices);
    return jointMatrices;
}

void AnimatedModel::addJointsToArray(Joint* headJoint, std::vector<Matrix4f>* jointMatrices)
{
    jointMatrices->at(headJoint->index) = headJoint->animatedTransform;
    for (Joint* childJoint : headJoint->children)
    {
        addJointsToArray(childJoint, jointMatrices);
    }
}
