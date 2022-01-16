#include <string>
#include <vector>

#include "geometryloader.hpp"
#include "../toolbox/vector.hpp"
#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"
#include "vertex2.hpp"
#include "../main/main.hpp"

GeometryLoader::GeometryLoader(
    std::vector<std::string> vertexPositionsRaw,
    std::vector<std::string> textureCoordsRaw,
    std::vector<std::string> normalsRaw,
    std::vector<std::string> vertexColorsRaw,
    std::vector<std::string> indicesRaw,
    std::vector<std::string> boneWeightsRaw,
    std::vector<VertexSkinData> vertexWeights)
{
    this->vertexPositionsRaw = vertexPositionsRaw;
    this->textureCoordsRaw   = textureCoordsRaw;
    this->normalsRaw         = normalsRaw;
    this->vertexColorsRaw    = vertexColorsRaw;
    this->indicesRaw         = indicesRaw;
    this->boneWeightsRaw     = boneWeightsRaw;
    this->vertexWeights      = vertexWeights;

    this->vertexPositionsRaw.erase(this->vertexPositionsRaw.begin());
    this->textureCoordsRaw  .erase(this->textureCoordsRaw.begin());
    this->normalsRaw        .erase(this->normalsRaw.begin());
    this->vertexColorsRaw   .erase(this->vertexColorsRaw.begin());
    this->indicesRaw        .erase(this->indicesRaw.begin());
    this->boneWeightsRaw    .erase(this->boneWeightsRaw.begin());
}

MeshData GeometryLoader::extractModelData()
{
    readRawData();
    assembleVertices();
    removeUnusedVertices();
    initArrays();
    convertDataToArrays();
    convertIndicesListToArray();
    return MeshData(verticesArray, texturesArray, normalsArray, vcolorsArray, indicesArray, jointIdsArray, weightsArray);
}

void GeometryLoader::readRawData()
{
    readPositions();
    readNormals();
    readTextureCoords();
}

void GeometryLoader::readPositions()
{
    int count = (int)vertexPositionsRaw.size();
    for (int i = 0; i < count/3; i++)
    {
        float x = std::stof(vertexPositionsRaw[i*3 + 0]);
        float y = std::stof(vertexPositionsRaw[i*3 + 1]);
        float z = std::stof(vertexPositionsRaw[i*3 + 2]);
        Vector4f pos(x, y, z, 1);

        Matrix4f CORRECTION;
        Vector3f xAxis(1, 0, 0);
        CORRECTION.rotate(Maths::toRadians(-90), &xAxis);

        Vector4f posCorrected = CORRECTION.transform(&pos);

        Vector3f pos2(posCorrected.x, posCorrected.y, posCorrected.z);
        Vertex2* newV = new Vertex2((int)vertices.size(), &pos2, vertexWeights[(int)vertices.size()]); INCR_NEW("Vertex2");
        vertices.push_back(newV);
    }
}

void GeometryLoader::readNormals()
{
    int count = (int)normalsRaw.size();
    for (int i = 0; i < count/3; i++)
    {
        float x = std::stof(normalsRaw[i*3 + 0]);
        float y = std::stof(normalsRaw[i*3 + 1]);
        float z = std::stof(normalsRaw[i*3 + 2]);
        Vector4f norm(x, y, z, 0.0f);
        
        Matrix4f CORRECTION;
        Vector3f xAxis(1, 0, 0);
        CORRECTION.rotate(Maths::toRadians(-90), &xAxis);

        Vector4f normCorrect = CORRECTION.transform(&norm);

        normals.push_back(Vector3f(normCorrect.x, normCorrect.y, normCorrect.z));
    }
}

void GeometryLoader::readTextureCoords()
{
    int count = (int)textureCoordsRaw.size();
    for (int i = 0; i < count/2; i++)
    {
        float s = std::stof(textureCoordsRaw[i*2 + 0]);
        float t = std::stof(textureCoordsRaw[i*2 + 1]);
        textures.push_back(Vector2f(s, t));
    }
}

void GeometryLoader::assembleVertices()
{
    int count = (int)indicesRaw.size();
    int typeCount = 4; //assuming that vertex position, texture coord, normals, and vertex color all are exported by blender
    for (int i = 0; i < count/typeCount; i++)
    {
        int positionIndex = std::stoi(indicesRaw[i*typeCount + 0]);
        int normalIndex   = std::stoi(indicesRaw[i*typeCount + 1]);
        int texCoordIndex = std::stoi(indicesRaw[i*typeCount + 2]);
        processVertex(positionIndex, normalIndex, texCoordIndex);
    }
}

Vertex2* GeometryLoader::processVertex(int posIndex, int normIndex, int texIndex)
{
    Vertex2* currentVertex = vertices[posIndex];
    if (!currentVertex->isSet())
    {
        currentVertex->setTextureIndex(texIndex);
        currentVertex->setNormalIndex(normIndex);
        indices.push_back(posIndex);
        return currentVertex;
    }
    else
    {
        return dealWithAlreadyProcessedVertex(currentVertex, texIndex, normIndex);
    }
}

std::vector<int> GeometryLoader::convertIndicesListToArray()
{
    for (int i = 0; i < indices.size(); i++)
    {
        indicesArray.push_back(indices[i]);
    }
    return indicesArray;
}

float GeometryLoader::convertDataToArrays()
{
    float furthestPoint = 0;
    for (int i = 0; i < vertices.size(); i++)
    {
        Vertex2* currentVertex = vertices[i];
        if (currentVertex->getLength() > furthestPoint)
        {
            furthestPoint = currentVertex->getLength();
        }
        Vector3f position = currentVertex->getPosition();
        Vector2f textureCoord = textures[currentVertex->getTextureIndex()];
		Vector3f normalVector = normals[currentVertex->getNormalIndex()];
		verticesArray[i * 3 + 0] = position.x;
		verticesArray[i * 3 + 1] = position.y;
		verticesArray[i * 3 + 2] = position.z;
		texturesArray[i * 2 + 0] = textureCoord.x;
		texturesArray[i * 2 + 1] = 1 - textureCoord.y;
		normalsArray [i * 3 + 0] = normalVector.x;
		normalsArray [i * 3 + 1] = normalVector.y;
		normalsArray [i * 3 + 2] = normalVector.z;
		VertexSkinData weights = currentVertex->weightsData;
		jointIdsArray[i * 3 + 0] = weights.jointIds[0];
		jointIdsArray[i * 3 + 1] = weights.jointIds[1];
		jointIdsArray[i * 3 + 2] = weights.jointIds[2];
		weightsArray [i * 3 + 0] = weights.weights[0];
		weightsArray [i * 3 + 1] = weights.weights[1];
		weightsArray [i * 3 + 2] = weights.weights[2];
    }
    return furthestPoint;
}

Vertex2* GeometryLoader::dealWithAlreadyProcessedVertex(Vertex2* previousVertex, int newTextureIndex, int newNormalIndex)
{
    if (previousVertex->hasSameTextureAndNormal(newTextureIndex, newNormalIndex))
    {
	    indices.push_back(previousVertex->getIndex());
	    return previousVertex;
    }
    else
    {
		Vertex2* anotherVertex = previousVertex->getDuplicateVertex();
		if (anotherVertex != nullptr)
        {
			return dealWithAlreadyProcessedVertex(anotherVertex, newTextureIndex, newNormalIndex);
		}
        else
        {
			Vertex2* duplicateVertex = new Vertex2((int)vertices.size(), previousVertex->getPosition(), previousVertex->weightsData); INCR_NEW("Vertex2");
			duplicateVertex->setTextureIndex(newTextureIndex);
			duplicateVertex->setNormalIndex(newNormalIndex);
			previousVertex->setDuplicateVertex(duplicateVertex);
			vertices.push_back(duplicateVertex);
			indices.push_back(duplicateVertex->getIndex());
			return duplicateVertex;
		}
	}
}

void GeometryLoader::initArrays()
{
    for (int i = 0; i < (int)vertices.size()*3; i++) { verticesArray.push_back(0.0f); }
    for (int i = 0; i < (int)vertices.size()*2; i++) { texturesArray.push_back(0.0f); }
    for (int i = 0; i < (int)vertices.size()*3; i++) { normalsArray .push_back(0.0f); }
    for (int i = 0; i < (int)vertices.size()*3; i++) { jointIdsArray.push_back(0);    }
    for (int i = 0; i < (int)vertices.size()*3; i++) { weightsArray .push_back(0.0f); }
}

void GeometryLoader::removeUnusedVertices()
{
    for (Vertex2* vertex : vertices)
    {
        vertex->averageTangents();
        if (!vertex->isSet())
        {
            vertex->setTextureIndex(0);
            vertex->setNormalIndex(0);
        }
    }
}
