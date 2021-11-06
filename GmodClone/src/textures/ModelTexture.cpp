#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <unordered_set>

#include "modeltexture.hpp"
#include "../renderEngine/renderEngine.hpp"
#include "../toolbox/maths.hpp"

std::unordered_set<ModelTexture*> ModelTexture::animatedTextureReferences;

ModelTexture::ModelTexture()
{
    shineDamper = 20.0f;
    reflectivity = 0.0f;
    hasTransparency = false;
    useFakeLighting = false;
    glowAmount = 0;
    isAnimated = false;
    animatedProgress = 0.0f;
    animationSpeed = 0.0f;
    currentImageIndex = 0;
    mixingType = 1;
    fogScale = 1.0f;
    renderOrder = 0;
}

ModelTexture::ModelTexture(std::vector<GLuint>* texIDs)
{
    for (GLuint id : (*texIDs))
    {
        this->texIDs.push_back(id);
    }
    shineDamper = 20.0f;
    reflectivity = 0.0f;
    hasTransparency = false;
    useFakeLighting = false;
    glowAmount = 0;
    isAnimated = false;
    animatedProgress = 0.0f;
    animationSpeed = 0.0f;
    currentImageIndex = 0;
    mixingType = 1;
    scrollX = 0.0f;
    scrollY = 0.0f;
    fogScale = 1.0f;
    renderOrder = 0;

    if (this->texIDs.size() > 1)
    {
        isAnimated = true;
    }
}

ModelTexture::ModelTexture(ModelTexture* other)
{
    std::vector<GLuint>* otherIDs = other->getIDs();
    for (GLuint id : (*otherIDs))
    {
        texIDs.push_back(id);
    }
    shineDamper         = other->shineDamper;
    reflectivity        = other->reflectivity;
    hasTransparency     = other->hasTransparency;
    useFakeLighting     = other->useFakeLighting;
    glowAmount          = other->glowAmount;
    isAnimated          = other->isAnimated;
    animatedProgress    = other->animatedProgress;
    animationSpeed      = other->animationSpeed;
    currentImageIndex   = other->currentImageIndex;
    mixingType          = other->mixingType;
    scrollX             = other->scrollX;
    scrollY             = other->scrollY;
    fogScale            = other->fogScale;
    renderOrder         = other->renderOrder;

    if (texIDs.size() > 1)
    {
        isAnimated = true;
    }
}

GLuint ModelTexture::getID()
{
    return texIDs[currentImageIndex];
}

GLuint ModelTexture::getID2()
{
    return texIDs[(currentImageIndex+1)%texIDs.size()];
}

bool ModelTexture::hasMultipleImages()
{
    return isAnimated;
}

void ModelTexture::deleteMe()
{
    for (GLuint id : texIDs)
    {
        Loader::deleteTexture(id);
    }
    texIDs.clear();
    ModelTexture::animatedTextureReferences.erase(this);
}

float ModelTexture::mixFactor()
{
    switch (mixingType)
    {
        case 1:
            return 0.0f;

        case 2:
            return animatedProgress;

        case 3:
            return 0.5f*(sinf(Maths::PI*(animatedProgress - 0.5f)) + 1);
    }

    return 0.0f;
}

std::vector<GLuint>* ModelTexture::getIDs()
{
    return &texIDs;
}

void ModelTexture::addMeToAnimationsSetIfNeeded()
{
    if (isAnimated)
    {
        ModelTexture::animatedTextureReferences.insert(this);
    }
}

void ModelTexture::updateAnimations(float dt)
{
    for (ModelTexture* tex : ModelTexture::animatedTextureReferences)
    {
        tex->animatedProgress += tex->animationSpeed*dt;
        if (tex->animatedProgress >= 1.0f)
        {
            tex->currentImageIndex = (tex->currentImageIndex+1)%tex->texIDs.size();
            tex->animatedProgress-=1.0f;
        }
    }
}

bool ModelTexture::equalTo(ModelTexture* other)
{
    return (
        std::equal(texIDs.begin(), texIDs.end(), other->getIDs()->begin()) &&
        isAnimated      == other->isAnimated      &&
        shineDamper     == other->shineDamper     &&
        reflectivity    == other->reflectivity    &&
        scrollX         == other->scrollX         &&
        scrollY         == other->scrollY         &&
        glowAmount      == other->glowAmount      &&
        hasTransparency == other->hasTransparency &&
        useFakeLighting == other->useFakeLighting &&
        fogScale        == other->fogScale        &&
        mixingType      == other->mixingType      &&
        animationSpeed  == other->animationSpeed  &&
        renderOrder     == other->renderOrder);
}
