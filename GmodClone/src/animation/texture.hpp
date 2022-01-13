#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class Texture
{
public:
    GLuint textureId;
    int size;
    int type;

    Texture(GLuint textureId, int size);

    Texture(GLuint textureId, int type, int size);

    void bindToUnit(int unit);

    void deleteMe();

    //static TextureBuilder newTexture(char* textureFilename);
};
#endif
