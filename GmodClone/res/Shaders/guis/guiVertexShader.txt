#version 140

in vec2 position;

out vec2 textureCoords;

uniform mat4 transformationMatrix;

void main(void)
{
    vec4 pos = transformationMatrix * vec4(position, 0.0, 1.0);
	gl_Position = vec4(2*pos.x - 1, -2*pos.y + 1, pos.z, pos.w);
	textureCoords = position;
}
