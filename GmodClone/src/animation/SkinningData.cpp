#include <string>
#include <vector>

#include "skinningdata.hpp"
#include "vertexskindata.hpp"

SkinningData::SkinningData(
    std::vector<std::string> jointOrder,
    std::vector<VertexSkinData> verticesSkinData)
{
    this->jointOrder = jointOrder;
    this->verticesSkinData = verticesSkinData;
}
