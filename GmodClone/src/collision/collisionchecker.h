#ifndef COLLISIONCHECKER_H
#define COLLISIONCHECKER_H

class Triangle3D;

#include <vector>
#include "../toolbox/vector.h"

class CollisionResult
{
public:
    bool hit = false;
    Vector3f directionToPosition;
    float distanceToPosition = 0.0f;
    Triangle3D* tri = nullptr;
};

class CollisionChecker
{
private:
    static Vector3f collidePosition;
    static Triangle3D* collideTriangle;
    static std::vector<Triangle3D*> triangles;

    static float chunkedTrianglesMinX;
    static float chunkedTrianglesMinZ;
    static float chunkedTrianglesChunkSize;
    static int chunkedTrianglesWidth;
    static int chunkedTrianglesHeight;
    static std::vector<std::vector<Triangle3D*>*> mapOfTriangles;

    static std::vector<Triangle3D*>* getTriangleChunk(float x, float z);

    static void addChunkToDataStruct(std::vector<Triangle3D*>* chunkToAdd, std::vector<std::vector<Triangle3D*>*>* chunksToCheck);

public:
    static CollisionResult checkCollision(Vector3f* p1, float sphereRadius);

    static CollisionResult checkCollision(float x, float y, float z, float sphereRadius);

    static void deleteAllTriangles();

    static void addTriangle(Triangle3D* cm);

    //based off of the last collision check
    static Triangle3D* getCollideTriangle();

    //based off of the last collision check
    static Vector3f* getCollidePosition();

    static void calculateDatastructure();
};

#endif
