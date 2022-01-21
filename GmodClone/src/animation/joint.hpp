#ifndef JOINT_H
#define JOINT_H

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class Joint
{
private:
    Matrix4f localBindTransform;

public:
    int index = -1;
    std::string name;
    std::vector<Joint*> children;

    Matrix4f animatedTransform;
    Matrix4f inverseBindTransform;
    
    Joint(int index, std::string name, Matrix4f* bindLocalTransform);

    void calcInverseBindTransform(Matrix4f* parentBindTransform);

    void deleteChildren();
};
#endif
