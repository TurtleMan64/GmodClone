#include <iostream>

#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "vertex2.hpp"
#include "vertexskindata.hpp"

Vertex2::Vertex2(int index, Vector3f* position, VertexSkinData weightsData)
{
    this->index = index;
    this->color.set(1.0f, 1.0f, 1.0f, 1.0f);
    this->position.set(position->x, position->y, position->z);
    this->length = position->length();
    this->textureIndex = NO_INDEX;
    this->normalIndex = NO_INDEX;
    this->duplicateVertex = nullptr;
    this->weightsData = weightsData;
}

Vertex2::Vertex2(int index, Vector3f* position, Vector4f* color)
{
    this->index = index;
    this->color.set(color);
    this->position.set(position->x, position->y, position->z);
    this->length = position->length();
    this->textureIndex = NO_INDEX;
    this->normalIndex = NO_INDEX;
    this->duplicateVertex = nullptr;
}

Vertex2::Vertex2(int index, Vector3f* position)
{
    this->index = index;
    //this->color.set(Maths::nextUniform(), Maths::nextUniform(), Maths::nextUniform());
    this->color.set(1.0f, 1.0f, 1.0f, 1.0f);
    this->position.set(position->x, position->y, position->z);
    this->length = position->length();
    this->textureIndex = NO_INDEX;
    this->normalIndex = NO_INDEX;
    this->duplicateVertex = nullptr;
}

void Vertex2::addTangent(Vector3f tangent)
{
    tangents.push_back(tangent);
}

void Vertex2::averageTangents()
{
    if (tangents.size() == 0)
    {
        return;
    }

    for (Vector3f tan : tangents)
    {
        averagedTangent = averagedTangent + tan;
    }

    averagedTangent.normalize();
}

int Vertex2::getIndex()
{
    return index;
}

float Vertex2::getLength()
{
    return length;
}

int Vertex2::isSet()
{
    if ((textureIndex != NO_INDEX) && (normalIndex != NO_INDEX))
    {
        return 1;
    }
    return 0;
}

int Vertex2::hasSameTextureAndNormal(int textureIndexOther, int normalIndexOther)
{
    if ((textureIndexOther == textureIndex) && (normalIndexOther == normalIndex))
    {
        return 1;
    }
    return 0;
}

void Vertex2::setTextureIndex(int newTextureIndex)
{
    textureIndex = newTextureIndex;
}

void Vertex2::setNormalIndex(int newNormalIndex)
{
    normalIndex = newNormalIndex;
}

Vector3f* Vertex2::getPosition()
{
    return &position;
}

int Vertex2::getTextureIndex()
{
    return textureIndex;
}

int Vertex2::getNormalIndex()
{
    return normalIndex;
}

Vertex2* Vertex2::getDuplicateVertex()
{
    return duplicateVertex;
}

void Vertex2::setDuplicateVertex(Vertex2* duplicateVertexNew)
{
    duplicateVertex = duplicateVertexNew;
}
