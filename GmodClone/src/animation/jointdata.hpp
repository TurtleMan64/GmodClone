#ifndef JOINT_DATA_H
#define JOINT_DATA_H

#include <string>
#include <vector>

#include "../toolbox/matrix.hpp"

class JointData
{
public:
    int index;
    std::string nameId;
    Matrix4f bindLocalTransform;

    std::vector<JointData*> children;

    JointData();

    JointData(int index, std::string nameId, Matrix4f bindLocalTransform);
};
#endif
