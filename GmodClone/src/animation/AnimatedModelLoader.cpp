#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "animatedmodelloader.hpp"
#include "animatedmodel.hpp"
#include "animatedmodeldata.hpp"
#include "meshdata.hpp"
#include "jointdata.hpp"
#include "joint.hpp"
#include "skeletondata.hpp"
#include "../openglObjects/vao.hpp"
#include "../toolbox/split.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../main/main.hpp"
#include "skinloader.hpp"
#include "skinningdata.hpp"
#include "meshdata.hpp"
#include "geometryloader.hpp"

AnimatedModel* AnimatedModelLoader::loadAnimatedModel(const char* folder, const char* filename)
{
    return AnimatedModelLoader::loadAnimatedModel((char*)folder, (char*)filename);
}

AnimatedModel* AnimatedModelLoader::loadAnimatedModel(char* folder, char* filename)
{
    std::string fullFile = "";
    fullFile = fullFile + folder + filename;

    std::string line;
    std::ifstream myfile((Global::pathToEXE + fullFile).c_str());
    if (myfile.is_open())
    {
        getline(myfile, line);
        std::vector<std::string> textureTokens = split(line, ' ');
        std::string textureFilename = "";
        textureFilename = textureFilename + folder + textureTokens[1];
        GLuint textureId = Loader::loadTexture(textureFilename.c_str());

        getline(myfile, line);
        std::vector<std::string> vertexPositions = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> textureCoords = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> normals = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> vertexColors = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> indices = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> boneWeights = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> howManyBonesEachVertexIsLinkedTo = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> whichBonesEachVertexIsLinkedToAndTheirWeights = split(line, ' ');

        getline(myfile, line);
        std::vector<std::string> boneNames = split(line, ' ');

        std::unordered_map<std::string, Joint*> boneNameToJoint;
        std::unordered_map<std::string, int> boneNameToIndex;
        std::unordered_map<std::string, std::string> boneNameToParent;

        int jointCount = (int)boneNames.size() - 1;

        for (int i = 1; i < (int)boneNames.size(); i++)
        {
            boneNameToIndex[boneNames[i]] = i - 1;
        }

        Joint* rootJoint = nullptr;

        for (int i = 1; i < (int)boneNames.size(); i++)
        {
            getline(myfile, line);
            std::vector<std::string> boneTokens = split(line, ' ');

            float mat4[16];
            for (int f = 0; f < 16; f++)
            {
                if (i == 1) //root bone doesnt have a parent
                {
                    mat4[f] = std::stof(boneTokens[f + 1]);
                }
                else
                {
                    mat4[f] = std::stof(boneTokens[f + 2]);
                }
            }

            Matrix4f ma;
            ma.loadColumnFirst(mat4);

            if (i == 1)
            {
                Matrix4f CORRECTION;
                Vector3f xAxis(1, 0, 0);
                CORRECTION.rotate(Maths::toRadians(-90), &xAxis);

                CORRECTION.multiply(&ma, &ma);
            }

            Joint* newJoint = new Joint(boneNameToIndex[boneTokens[0]], boneTokens[0], &ma); INCR_NEW("Joint");

            if (i == 1)
            {
                rootJoint = newJoint;
            }

            boneNameToJoint[boneTokens[0]] = newJoint;
            if (i != 1) //root bone doesnt have a parent
            {
                boneNameToParent[boneTokens[0]] = boneTokens[1];
            }
        }

        //set up the parent child relationship
        for (auto const& entry : boneNameToParent)
        {
            Joint* parent = boneNameToJoint[entry.second];
            Joint* child  = boneNameToJoint[entry.first];
            parent->children.push_back(child);
        }

        Matrix4f identityMat;
        rootJoint->calcInverseBindTransform(&identityMat);

        myfile.close();

        SkinLoader skinLoader = SkinLoader(
            boneNames,
            boneWeights,
            howManyBonesEachVertexIsLinkedTo,
            whichBonesEachVertexIsLinkedToAndTheirWeights,
            3);
        SkinningData skinningData = skinLoader.extractSkinData();

        GeometryLoader gLoader = GeometryLoader(
            vertexPositions,
            textureCoords,
            normals,
            vertexColors,
            indices,
            boneWeights,
            skinningData.verticesSkinData);
        MeshData meshData = gLoader.extractModelData();

        Vao* model = createVao(&meshData);

        INCR_NEW("AnimatedModel");
        return new AnimatedModel(model, textureId, rootJoint, jointCount);
    }
    else
    {
        printf("Couldn't load animation mesh file '%s'\n", (Global::pathToEXE + fullFile).c_str());
        return nullptr;
    }
}

Vao* AnimatedModelLoader::createVao(MeshData* data)
{
    Vao* vao = Vao::create();
    std::vector<GLuint> blank;
    vao->bind(&blank);
    vao->createIndexBuffer(&data->indices);
    vao->createAttribute   (0, &data->vertices,      3);
    vao->createAttribute   (1, &data->textureCoords, 2);
    vao->createAttribute   (2, &data->normals,       3);
    vao->createIntAttribute(3, &data->jointIds,      3);
    vao->createAttribute   (4, &data->vertexWeights, 3);
    vao->unbind(&blank);

    //printf("indices size = %d\n", (int)data->indices.size());
    //for (int i = 0; i < (int)data->indices.size(); i++)
    //{
    //    printf("%d ", data->indices[i]);
    //}
    //printf("\n");
    //
    //printf("vertices size = %d\n", (int)data->vertices.size());
    //for (int i = 0; i < (int)data->vertices.size(); i++)
    //{
    //    printf("%f ", data->vertices[i]);
    //}
    //printf("\n");
    //
    //printf("textureCoords size = %d\n", (int)data->textureCoords.size());
    //for (int i = 0; i < (int)data->textureCoords.size(); i++)
    //{
    //    printf("%f ", data->textureCoords[i]);
    //}
    //printf("\n");
    //
    //printf("normals size = %d\n", (int)data->normals.size());
    //for (int i = 0; i < (int)data->normals.size(); i++)
    //{
    //    printf("%f ", data->normals[i]);
    //}
    //printf("\n");
    //
    //printf("jointIds size = %d\n", (int)data->jointIds.size());
    //for (int i = 0; i < (int)data->jointIds.size(); i++)
    //{
    //    printf("%d ", data->jointIds[i]);
    //}
    //printf("\n");
    //
    //printf("vertexWeights size = %d\n", (int)data->vertexWeights.size());
    //for (int i = 0; i < (int)data->vertexWeights.size(); i++)
    //{
    //    printf("%f ", data->vertexWeights[i]);
    //}
    //printf("\n");

    //printf("indices size = %d\n", (int)data->indices.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%d ", data->indices[i]);
    //}
    //printf("\n");
    //
    //printf("vertices size = %d\n", (int)data->vertices.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%f ", data->vertices[i]);
    //}
    //printf("\n");
    //
    //printf("textureCoords size = %d\n", (int)data->textureCoords.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%f ", data->textureCoords[i]);
    //}
    //printf("\n");
    //
    //printf("normals size = %d\n", (int)data->normals.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%f ", data->normals[i]);
    //}
    //printf("\n");
    //
    //printf("jointIds size = %d\n", (int)data->jointIds.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%d ", data->jointIds[i]);
    //}
    //printf("\n");
    //
    //printf("vertexWeights size = %d\n", (int)data->vertexWeights.size());
    //for (int i = 0; i < 10; i++)
    //{
    //    printf("%f ", data->vertexWeights[i]);
    //}
    //printf("\n");

    //printf("indices size = %d\n", (int)data->indices.size());
    //for (int i = (int)data->indices.size() - 10; i < (int)data->indices.size(); i++)
    //{
    //    printf("%d ", data->indices[i]);
    //}
    //printf("\n");
    //
    //printf("vertices size = %d\n", (int)data->vertices.size());
    //for (int i = (int)data->vertices.size() - 10; i < (int)data->vertices.size(); i++)
    //{
    //    printf("%f ", data->vertices[i]);
    //}
    //printf("\n");
    //
    //printf("textureCoords size = %d\n", (int)data->textureCoords.size());
    //for (int i = (int)data->textureCoords.size() - 10; i < (int)data->textureCoords.size(); i++)
    //{
    //    printf("%f ", data->textureCoords[i]);
    //}
    //printf("\n");
    //
    //printf("normals size = %d\n", (int)data->normals.size());
    //for (int i = (int)data->normals.size() - 10; i < (int)data->normals.size(); i++)
    //{
    //    printf("%f ", data->normals[i]);
    //}
    //printf("\n");
    //
    //printf("jointIds size = %d\n", (int)data->jointIds.size());
    //for (int i = (int)data->jointIds.size() - 10; i < (int)data->jointIds.size(); i++)
    //{
    //    printf("%d ", data->jointIds[i]);
    //}
    //printf("\n");
    //
    //printf("vertexWeights size = %d\n", (int)data->vertexWeights.size());
    //for (int i = (int)data->vertexWeights.size() - 10; i < (int)data->vertexWeights.size(); i++)
    //{
    //    printf("%f ", data->vertexWeights[i]);
    //}
    //printf("\n");

    return vao;
}
