#ifndef OBJLOADER_H
#define OBJLOADER_H

class CollisionModel;
class Vertex;
class QuadTreeNode;

#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdio.h>

#include "../textures/modeltexture.hpp"
#include "../toolbox/vector.hpp"
#include "../models/models.hpp"

class ObjLoader
{
private:
    static void parseMtl(std::string filePath, std::string fileName, std::unordered_map<std::string, ModelTexture>* outMtlMap);

    static void deleteUnusedMtl(std::unordered_map<std::string, ModelTexture>* mtlMap, std::vector<ModelTexture>* usedMtls);

    static Vertex* processVertex(char** vertex,
        std::vector<Vertex*>* vertices,
        std::vector<int>* indices);

    static Vertex* processVertexBinary(int, int, int,
        std::vector<Vertex*>* vertices,
        std::vector<int>* indices);

    static Vertex* dealWithAlreadyProcessedVertex(Vertex*,
        int,
        int,
        std::vector<int>*,
        std::vector<Vertex*>*);

    static void removeUnusedVertices(std::vector<Vertex*>* vertices);

    static void convertDataToArrays(
        std::vector<Vertex*>* vertices,
        std::vector<Vector2f>* textures,
        std::vector<Vector3f>* normals,
        std::vector<float>* verticesArray,
        std::vector<float>* texturesArray,
        std::vector<float>* normalsArray,
        std::vector<float>* colorsArray,
        std::vector<float>* tangentsArray);

    static void calculateTangents(Vertex* v0, Vertex* v1, Vertex* v2, std::vector<Vector2f>* textures);

public:
    //Attempts to load a mode as either an OBJ or binary format.
    //Checks for binary file first, then tries OBJ.
    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadModel(Model* models, std::string filePath, std::string fileName);

private:
    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadObjModel(Model* models, std::string filePath, std::string fileName);

    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadBinaryModel(Model* models, std::string filePath, std::string fileName);

    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadVclModel(Model* models, std::string filePath, std::string fileName);

public:
    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadObjModelWithMTL(Model* models, std::string filePath, std::string fileNameOBJ, std::string fileNameMTL);

    //Each TexturedModel contained within 'models' must be deleted later.
    //Returns 0 if successful, 1 if model is already loaded, -1 if file couldn't be loaded
    static int loadBinaryModelWithMTL(Model* models, std::string filePath, std::string fileNameBin, std::string fileNameMTL);

    //The CollisionModel returned must be deleted later.
    static CollisionModel* loadCollisionModel(std::string filePath, std::string fileName);

    //The CollisionModel returned must be deleted later.
    static CollisionModel* loadBinaryCollisionModel(std::string filePath, std::string fileName);
};
#endif
