#ifndef VERTEX_2_H
#define VERTEX_2_H

#include "../toolbox/vector.hpp"
#include <vector>
#include "vertexskindata.hpp"

class Vertex2
{
public:
    const int NO_INDEX = -1;

    Vector3f position;
    Vector4f color;
    int textureIndex = NO_INDEX;
    int normalIndex = NO_INDEX;
    Vertex2* duplicateVertex = nullptr;
    int index = -1;
    float length = 0.0f;
    std::vector<Vector3f> tangents;
    Vector3f averagedTangent;

    VertexSkinData weightsData;

public:
    Vertex2(int index, Vector3f* position, VertexSkinData weightsData);

    Vertex2(int index, Vector3f* position, Vector4f* color);

    Vertex2(int index, Vector3f* position);

    void addTangent(Vector3f tangent);

    void averageTangents();

    int getIndex();

    float getLength();

    int isSet();

    int hasSameTextureAndNormal(int textureIndexOther, int normalIndexOther);

    void setTextureIndex(int textureIndex);

    void setNormalIndex(int normalIndex);

    Vector3f* getPosition();

    int getTextureIndex();

    int getNormalIndex();

    Vertex2* getDuplicateVertex();

    void setDuplicateVertex(Vertex2* duplicateVertex);

};
#endif
