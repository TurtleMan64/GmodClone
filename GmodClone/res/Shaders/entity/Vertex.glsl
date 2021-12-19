#version 400 core

in vec3 position;
in vec2 textureCoords;
in vec3 normal;
in vec4 vertexColor;
in vec3 tangent;

out vec3 toCameraVector;
out vec3 toLightVector[4];
out vec2 pass_textureCoords;
out vec4 pass_vertexColor;

uniform mat4 transformationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 lightPositionEyeSpace[4];

uniform float useFakeLighting;

//for use in animation of the texture coordinates
uniform float texOffX;
uniform float texOffY;

//uniform vec4 clipPlane;
uniform vec4 clipPlaneBehind;

void main(void)
{
	vec4 worldPosition = transformationMatrix * vec4(position, 1.0);
    
    gl_ClipDistance[0] = dot(worldPosition, clipPlaneBehind);
    
    mat4 modelViewMatrix = viewMatrix * transformationMatrix;
	vec4 positionRelativeToCam = modelViewMatrix * vec4(position, 1.0);
    
	gl_Position = projectionMatrix * positionRelativeToCam;
	
    pass_textureCoords = textureCoords + vec2(texOffX, texOffY);

	vec3 surfaceNormal = (modelViewMatrix * vec4(normal, 0.0)).xyz;
    
    vec3 norm   = normalize(surfaceNormal);
    vec3 tang   = normalize((modelViewMatrix * vec4(tangent, 0.0)).xyz);
    vec3 bitang = normalize(cross(norm, tang));
	
	mat3 toTangentSpace = mat3(
		tang.x, bitang.x, norm.x,
		tang.y, bitang.y, norm.y,
		tang.z, bitang.z, norm.z
	);
    
    toLightVector[0] = toTangentSpace * (lightPositionEyeSpace[0] - positionRelativeToCam.xyz);
    toLightVector[1] = toTangentSpace * (lightPositionEyeSpace[1] - positionRelativeToCam.xyz);
    toLightVector[2] = toTangentSpace * (lightPositionEyeSpace[2] - positionRelativeToCam.xyz);
    toLightVector[3] = toTangentSpace * (lightPositionEyeSpace[3] - positionRelativeToCam.xyz);
	
	toCameraVector = toTangentSpace * (-positionRelativeToCam.xyz);

	pass_vertexColor = vertexColor;
}
