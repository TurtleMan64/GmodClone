#version 400 core

in vec2 position;

out vec4 clipSpace;
out vec2 textureCoords;
out vec3 toCameraVector;
//out vec3 worldPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;
//uniform vec3 lightPosition;
//uniform mat4 toShadowMapSpace;
uniform vec4 clipPlaneBehind;

void main(void) 
{
    vec4 worldPosition = modelMatrix * vec4(position.x, 0, position.y, 1);
    gl_ClipDistance[1] = dot(worldPosition, clipPlaneBehind);
    clipSpace = projectionMatrix * viewMatrix * worldPosition;
    gl_Position = clipSpace;
    textureCoords = vec2(worldPosition.x, worldPosition.z)*0.25;
    toCameraVector = cameraPosition - worldPosition.xyz;
}
