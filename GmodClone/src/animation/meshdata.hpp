#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <vector>

class MeshData
{
public:
    static constexpr int DIMENSIONS = 3;

    std::vector<float> vertices;
    std::vector<float> textureCoords;
    std::vector<float> normals;
    std::vector<float> vertexColors;
    std::vector<int> indices;
    std::vector<int> jointIds;
    std::vector<float> vertexWeights;

    MeshData();

    MeshData(std::vector<float> vertices,
             std::vector<float> textureCoords,
             std::vector<float> normals,
             std::vector<float> vertexColors,
             std::vector<int> indices,
             std::vector<int> jointIds,
             std::vector<float> vertexWeights);

    int getVertexCount();
};
#endif
