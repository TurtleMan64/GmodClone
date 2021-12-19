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
    normalMapId = GL_NONE;
    shineDamper = 20.0f;
    reflectivity = 0.0f;
    hasTransparency = false;
    useFakeLighting = false;
    glowAmount = 0;
    noise = 1.0f;
    isAnimated = false;
    animatedProgress = 0.0f;
    animationSpeed = 0.0f;
    currentImageIndex = 0;
    mixingType = 1;
    fogScale = 1.0f;
    renderOrder = 0;
}

ModelTexture::ModelTexture(std::vector<GLuint>* texIds)
{
    for (GLuint id : (*texIds))
    {
        this->texIds.push_back(id);
    }
    normalMapId = GL_NONE;
    shineDamper = 20.0f;
    reflectivity = 0.0f;
    hasTransparency = false;
    useFakeLighting = false;
    glowAmount = 0;
    noise = 1.0f;
    isAnimated = false;
    animatedProgress = 0.0f;
    animationSpeed = 0.0f;
    currentImageIndex = 0;
    mixingType = 1;
    scrollX = 0.0f;
    scrollY = 0.0f;
    fogScale = 1.0f;
    renderOrder = 0;

    if (this->texIds.size() > 1)
    {
        isAnimated = true;
    }
}

ModelTexture::ModelTexture(ModelTexture* other)
{
    std::vector<GLuint>* otherIds = other->getIds();
    for (GLuint id : (*otherIds))
    {
        texIds.push_back(id);
    }
    normalMapId         = other->normalMapId;
    shineDamper         = other->shineDamper;
    reflectivity        = other->reflectivity;
    hasTransparency     = other->hasTransparency;
    useFakeLighting     = other->useFakeLighting;
    glowAmount          = other->glowAmount;
    noise               = other->noise;
    isAnimated          = other->isAnimated;
    animatedProgress    = other->animatedProgress;
    animationSpeed      = other->animationSpeed;
    currentImageIndex   = other->currentImageIndex;
    mixingType          = other->mixingType;
    scrollX             = other->scrollX;
    scrollY             = other->scrollY;
    fogScale            = other->fogScale;
    renderOrder         = other->renderOrder;

    if (texIds.size() > 1)
    {
        isAnimated = true;
    }
}

GLuint ModelTexture::getId()
{
    return texIds[currentImageIndex];
}

GLuint ModelTexture::getId2()
{
    return texIds[(currentImageIndex+1)%texIds.size()];
}

bool ModelTexture::hasMultipleImages()
{
    return isAnimated;
}

void ModelTexture::deleteMe()
{
    for (GLuint id : texIds)
    {
        Loader::deleteTexture(id);
    }
    texIds.clear();
    if (normalMapId != GL_NONE)
    {
        Loader::deleteTexture(normalMapId);
        normalMapId = GL_NONE;
    }
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

std::vector<GLuint>* ModelTexture::getIds()
{
    return &texIds;
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
            tex->currentImageIndex = (tex->currentImageIndex+1)%tex->texIds.size();
            tex->animatedProgress-=1.0f;
        }
    }
}

bool ModelTexture::equalTo(ModelTexture* other)
{
    return (
        std::equal(texIds.begin(), texIds.end(), other->getIds()->begin()) &&
        isAnimated      == other->isAnimated      &&
        shineDamper     == other->shineDamper     &&
        reflectivity    == other->reflectivity    &&
        scrollX         == other->scrollX         &&
        scrollY         == other->scrollY         &&
        glowAmount      == other->glowAmount      &&
        noise           == other->noise           &&
        hasTransparency == other->hasTransparency &&
        useFakeLighting == other->useFakeLighting &&
        fogScale        == other->fogScale        &&
        mixingType      == other->mixingType      &&
        animationSpeed  == other->animationSpeed  &&
        renderOrder     == other->renderOrder     &&
        normalMapId     == other->normalMapId);
}
