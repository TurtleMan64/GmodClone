#ifndef GEOMETRY_LOADER_H
#define GEOMETRY_LOADER_H

#include <string>
#include <vector>

#include "vertexskindata.hpp"
#include "meshdata.hpp"
#include "vertex2.hpp"
#include "../toolbox/vector.hpp"

class GeometryLoader
{
public:
    GeometryLoader(
        std::vector<std::string> vertexPositionsRaw,
        std::vector<std::string> textureCoordsRaw,
        std::vector<std::string> normalsRaw,
        std::vector<std::string> vertexColorsRaw,
        std::vector<std::string> indicesRaw,
        std::vector<std::string> boneWeightsRaw,
        std::vector<VertexSkinData> vertexWeights);

    MeshData extractModelData();

private:
    std::vector<std::string> vertexPositionsRaw;
    std::vector<std::string> textureCoordsRaw;
    std::vector<std::string> normalsRaw;
    std::vector<std::string> vertexColorsRaw;
    std::vector<std::string> indicesRaw;
    std::vector<std::string> boneWeightsRaw;

    std::vector<VertexSkinData> vertexWeights;

    std::vector<float> verticesArray;
    std::vector<float> texturesArray;
    std::vector<float> normalsArray;
    std::vector<float> vcolorsArray;
    std::vector<int>   indicesArray;
    std::vector<int>   jointIdsArray;
    std::vector<float> weightsArray;

    std::vector<Vertex2*> vertices;
    std::vector<Vector2f> textures;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> vcolors;
    std::vector<int>   indices;

    void readRawData();

    void readPositions();

    void readNormals();

    void readTextureCoords();

    void assembleVertices();

    Vertex2* processVertex(int posIndex, int normIndex, int texIndex);

    std::vector<int> convertIndicesListToArray();

    float convertDataToArrays();

    Vertex2* dealWithAlreadyProcessedVertex(Vertex2* previousVertex, int newTextureIndex, int newNormalIndex);

    void initArrays();

    void removeUnusedVertices();
};
#endif
