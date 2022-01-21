#include <string>

#include "joint.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "../main/main.hpp"

Joint::Joint(int index, std::string name, Matrix4f* bindLocalTransform)
{
    this->index = index;
    this->name = name;
    this->localBindTransform.set(bindLocalTransform);
}

void Joint::calcInverseBindTransform(Matrix4f* parentBindTransform)
{
    Matrix4f bindTransform;
    parentBindTransform->multiply(&localBindTransform, &bindTransform);

    inverseBindTransform.set(&bindTransform);
    inverseBindTransform.invert();

	for (Joint* child : children)
    {
        child->calcInverseBindTransform(&bindTransform);
	}
}

void Joint::deleteChildren()
{
    for (Joint* child : children)
    {
        child->deleteChildren();
        delete child; INCR_DEL("Joint");
	}
    children.clear();
}
