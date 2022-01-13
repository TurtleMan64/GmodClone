#include <vector>

#include "meshdata.hpp"

MeshData::MeshData()
{

}

MeshData::MeshData(
    std::vector<float> vertices,
    std::vector<float> textureCoords,
    std::vector<float> normals,
    std::vector<float> vertexColors,
    std::vector<int> indices,
    std::vector<int> jointIds,
    std::vector<float> vertexWeights)
{
    this->vertices = vertices;
	this->textureCoords = textureCoords;
	this->normals = normals;
    this->vertexColors = vertexColors;
	this->indices = indices;
	this->jointIds = jointIds;
	this->vertexWeights = vertexWeights;
}

int MeshData::getVertexCount()
{
    return (int)vertices.size() / DIMENSIONS;
}
