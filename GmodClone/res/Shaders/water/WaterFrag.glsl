#version 400 core

in vec4 clipSpace;
in vec2 textureCoords;
in vec3 toCameraVector;
in vec3 passCameraPosition;
//in vec3 fromLightVector;

in vec3 worldposition;

out vec4 out_Color;
out vec4 out_BrightColor;

uniform vec3 sunDirection;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform vec3 sunColor;
uniform float waterHeight;
uniform float moveFactor;
uniform float murkiness;
uniform vec3 waterColor;

const float NEAR = 0.05;
const float FAR = 3000.0;

const float waveStrength = 0.025;
const float shineDamper = 80.0;
const float reflectivity = 0.80;


bool pixelIsInBounds(sampler2D tex, ivec2 coords, ivec2 size, out vec4 color)
{
    if (coords.x < 0 || coords.x >= size.x ||
        coords.y < 0 || coords.y >= size.y)
    {
        return false;
    }
    
    color = texelFetch(tex, coords, 0);
    
    return !(color.x > 0.99 && color.y < 0.01 && color.z > 0.99);
}

void main(void)
{
    vec2 ndc = (clipSpace.xy/clipSpace.w)*0.5 + 0.5;
    vec2 refractTexCoords = vec2(ndc.x,     ndc.y);
    vec2 reflectTexCoords = vec2(ndc.x, 1 - ndc.y);
    
    
    float depth = texture(depthMap, refractTexCoords).r;

    //converts raw un-linear data from depthMap to the actual distance that it truly is 
    float floorDistance = (2.0*NEAR*FAR)/((FAR + NEAR) - ((2.0*depth) - 1.0)*(FAR - NEAR));
    
    depth = gl_FragCoord.z;
    float waterDistance = (2.0*NEAR*FAR)/((FAR + NEAR) - ((2.0*depth) - 1.0)*(FAR - NEAR));
    //float waterDepth = floorDistance - waterDistance;
    float waterDepthFactor = clamp((floorDistance - waterDistance)*8, 0.0, 1.0);
    
    vec2 rawDudvMapVal = texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg;
    vec2 distortedTexCoords = rawDudvMapVal*0.1;
    distortedTexCoords = textureCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
    
    vec2 rawDudvMapVal2 = texture(dudvMap, distortedTexCoords).rg;
    rawDudvMapVal2 = rawDudvMapVal2 + vec2(0.066, 0.066); // need to offset it a bit because the dudv map image is not averaged at 0,0
    
    vec2 totalDistortion = (rawDudvMapVal2 * 2.0 - 1.0) * waveStrength;//* clamp(waterDepth/20.0, 0.0, 1.0);
    
    vec3 viewVector = normalize(toCameraVector);
    float refractiveFactor = dot(viewVector, vec3(0, 1, 0));
    float steepness = abs(refractiveFactor);
    
    totalDistortion*=steepness;
    
    ivec2 tSize = textureSize(refractionTexture, 0);
    
    vec4 refractColor;
    vec4 reflectColor;
    
    float d = 0.0078125;
    
    if  (pixelIsInBounds(refractionTexture, ivec2((refractTexCoords + totalDistortion)*tSize), tSize, refractColor)) {}
    else if (pixelIsInBounds(refractionTexture, ivec2((refractTexCoords + vec2(-d, 0))*tSize), tSize, refractColor)) {}
    else if (pixelIsInBounds(refractionTexture, ivec2((refractTexCoords + vec2( d, 0))*tSize), tSize, refractColor)) {}
    else if (pixelIsInBounds(refractionTexture, ivec2((refractTexCoords + vec2(0, -d))*tSize), tSize, refractColor)) {}
    else if (pixelIsInBounds(refractionTexture, ivec2((refractTexCoords + vec2(0,  d))*tSize), tSize, refractColor)) {}
    else
    {
        refractColor = vec4(0, 1, 1, 1);
    }
    
    if  (pixelIsInBounds(reflectionTexture, ivec2((reflectTexCoords + totalDistortion)*tSize), tSize, reflectColor)) {}
    else if (pixelIsInBounds(reflectionTexture, ivec2((reflectTexCoords + vec2(-d, 0))*tSize), tSize, reflectColor)) {}
    else if (pixelIsInBounds(reflectionTexture, ivec2((reflectTexCoords + vec2( d, 0))*tSize), tSize, reflectColor)) {}
    else if (pixelIsInBounds(reflectionTexture, ivec2((reflectTexCoords + vec2(0, -d))*tSize), tSize, reflectColor)) {}
    else if (pixelIsInBounds(reflectionTexture, ivec2((reflectTexCoords + vec2(0,  d))*tSize), tSize, reflectColor)) {}
    else
    {
        reflectColor = vec4(0, 1, 1, 1);
    }
    
    //out_Color = reflectColor;
    //return;
    
    //refractColor*=0.85;
    
    if (passCameraPosition.y <= waterHeight)
    {
        refractiveFactor = -refractiveFactor*2;
        
        reflectColor = mix(reflectColor, vec4(waterColor, 1.0), waterDepthFactor*murkiness);
    }
    else
    {
        refractColor = mix(refractColor, vec4(waterColor, 1.0), waterDepthFactor*murkiness);
    }
    
    // adjust the mix factor
    refractiveFactor*=2.0;
    refractiveFactor = clamp(refractiveFactor, 0.4, 0.9);
    
    // combine the refract and reflect colors into final output
    out_Color = mix(reflectColor, refractColor, refractiveFactor);
    
    // specular highlights
    vec4 normalMapColor = texture(normalMap, distortedTexCoords);
    vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
    normal = normalize(normal);
    
    vec3 reflectedLight = reflect(sunDirection, normal);
    float specular = max(dot(reflectedLight, viewVector), 0.0);
    specular = pow(specular, shineDamper);

    vec3 specularHighlights = sunColor * specular * reflectivity; //* (waterDepthFactor;
    out_Color = out_Color + vec4(specularHighlights, 0.0);
    out_Color.a = waterDepthFactor;

    // bloom
    //float brightness = (out_Color.r * 0.2126) + (out_Color.g * 0.7152) + (out_Color.b * 0.0722);
    //out_BrightColor = vec4((out_Color * brightness * brightness * brightness * brightness).rgb, 1.0);
}
