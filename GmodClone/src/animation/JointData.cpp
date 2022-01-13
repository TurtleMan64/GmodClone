#include <string>
#include <vector>

#include "jointdata.hpp"
#include "../toolbox/matrix.hpp"

JointData::JointData()
{

}

JointData::JointData(int index, std::string nameId, Matrix4f bindLocalTransform)
{
    this->index = index;
    this->nameId = nameId;
    this->bindLocalTransform = bindLocalTransform;
}
