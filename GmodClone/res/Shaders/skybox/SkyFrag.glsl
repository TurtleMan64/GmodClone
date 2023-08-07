#version 400 core

in vec3 pass_textureCoords;
out vec4 out_Color;

uniform samplerCube cubeMap;

void main(void)
{
    out_Color = texture(cubeMap, pass_textureCoords);
    
    //out_Color = vec4(1, 1, 0, 1);
}
