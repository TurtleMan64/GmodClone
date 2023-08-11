#version 400 core

in vec3 position;
out vec3 pass_textureCoords;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec4 clipPlane;
uniform vec3 center;

void main(void)
{
    vec4 worldPosition = vec4(position + center, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldPosition;
    gl_ClipDistance[0] = dot(worldPosition, clipPlane);
    pass_textureCoords = position;
}
