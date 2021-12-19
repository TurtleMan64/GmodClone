#ifndef MODELTEXTURES_H
#define MODELTEXTURES_H

#include <glad/glad.h>
#include "../toolbox/vector.hpp"
#include <vector>
#include <unordered_set>

class ModelTexture
{
private:
    std::vector<GLuint> texIds;

    //animation
    bool isAnimated = false; //has more than 1 image
    char currentImageIndex = 0; //current index of the animation
    float animatedProgress = 0.0f; //progress to the next image in the animation

    //for use of updating the animation values
    static std::unordered_set<ModelTexture*> animatedTextureReferences;

public:
    GLuint normalMapId = GL_NONE;
    float shineDamper = 20.0f;
    float reflectivity = 0.0f;
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    float glowAmount = 0.0f;
    float noise = 1.0f;
    bool hasTransparency = false;
    bool useFakeLighting = false;
    float fogScale = 0.0f;
    int mixingType = 0; //interpolation. 1 = binary, 2 = linear, 3 = sinusoid
    float animationSpeed = 0.0f; //delta per second
    char renderOrder = 0; //0 = rendered first (default), 1 = second, 2 = third, 3 = fifth + transparent (no depth testing), 4 = fourth + no depth writing

    ModelTexture();

    ModelTexture(std::vector<GLuint>* texIds);

    ModelTexture(ModelTexture* other);

    bool hasMultipleImages();

    //returns image 1
    GLuint getId();

    //returns image 2 (next image in the animation)
    GLuint getId2();

    //how much the 2nd image should be mixed with the first (for animations)
    float mixFactor();

    std::vector<GLuint>* getIds();

    //deletes all of the texture Ids out of gpu memory
    void deleteMe();

    void addMeToAnimationsSetIfNeeded();

    //updates all of the textures animation progress by dt
    static void updateAnimations(float dt);

    bool equalTo(ModelTexture* other);
};
#endif
