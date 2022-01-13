#ifndef SKINNING_DATA_H
#define SKINNING_DATA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

#include "vertexskindata.hpp"

class SkinningData
{
public:
    std::vector<std::string> jointOrder;
    std::vector<VertexSkinData> verticesSkinData;

    SkinningData(
        std::vector<std::string> jointOrder,
        std::vector<VertexSkinData> verticesSkinData);
};
#endif
