#version 400 core

in vec3 toCameraVector;
in vec3 toLightVector[4];
in vec2 pass_textureCoords;
in vec4 pass_vertexColor;

out vec4 out_Color;

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;
//uniform sampler2D depthBufferTransparent;
uniform sampler2D normalMap;
uniform sampler2D randomMap;
//uniform int   isRenderingTransparent;
//uniform int   isRenderingDepth;
uniform vec3  lightColor[4];
uniform vec3  attenuation[4];
//uniform float shineDamper;
uniform float reflectivity;
uniform float useFakeLighting;
uniform vec3  skyColor;
uniform int   hasTransparency;
uniform float glowAmount;
uniform vec3  baseColor;
uniform float baseAlpha;
uniform float mixFactor;
//uniform vec3  waterColor;
//uniform float waterBlendAmount;

uniform float fogDensity;
uniform float fogGradient;
uniform float fogScale; //per material
uniform float fogBottomPosition;
uniform float fogBottomThickness;

uniform int clock;
uniform float noise;
uniform int entityId;

void main(void)
{
    if (noise < 1.0)
    {
        vec2 fragCoordNormalized = gl_FragCoord.xy / 1024.0; //1024 is size of the randomMap.png
        
        vec4 randomSample = texture(randomMap, fragCoordNormalized + vec2(clock + 3.333*entityId, clock - 3.333*entityId));
        float rn;
        switch (clock % 12)
        {
            case  0: rn =       randomSample.r;    break;
            case  1: rn =       randomSample.g;    break;
            case  2: rn =       randomSample.b;    break;
            case  3: rn =       randomSample.a;    break;
            case  4: rn = fract(randomSample.r*2); break;
            case  5: rn = fract(randomSample.g*2); break;
            case  6: rn = fract(randomSample.b*2); break;
            case  7: rn = fract(randomSample.a*2); break;
            case  8: rn = fract(randomSample.r*4); break;
            case  9: rn = fract(randomSample.g*4); break;
            case 10: rn = fract(randomSample.b*4); break;
            case 11: rn = fract(randomSample.a*4); break;
        }
        
        if (rn > noise)
        {
            discard;
        }
    }
    
    vec4 rawTextureColor = mix(texture(textureSampler, pass_textureCoords), texture(textureSampler2, pass_textureCoords), mixFactor);
    rawTextureColor.rgb *= baseColor*pass_vertexColor.rgb;
    rawTextureColor.a   *= baseAlpha*pass_vertexColor.a*0.5; //for some bizarre reason, 0.5 alpha is seen as full opaque...

    if (hasTransparency == 0)
    {
        if (rawTextureColor.a < 0.45)
        {
            discard;
        }
        rawTextureColor.a = 1;
    }
    else if (rawTextureColor.a == 0)
    {
        discard;
    }
    
    vec4 normalMapValue = 2.0 * texture(normalMap, pass_textureCoords, -1.0) - 1.0;
    
    vec3 unitNormal = normalize(vec3(normalMapValue.r, normalMapValue.g, normalMapValue.b + 1.0)); //bias it a little towards flat
    
    vec3 totalDiffuse = vec3(0);
    vec3 totalSpecular = vec3(0);
    
    for (int i = 0; i < 4; i++)
    {
        float dist = length(toLightVector[i]); //toLightVector is making things dark in eq. if the cam direction has 0 for y, it happens
        float attFactor = attenuation[i].x + (attenuation[i].y * dist) + (attenuation[i].z * dist * dist);
        
        vec3 unitLightVector = normalize(toLightVector[i]);
        
        float nDotl = dot(unitNormal, unitLightVector);
        float brightness = nDotl*0.5 + 0.5;  //two different types of lighting options
        
        //make full brightness if glow
        if (useFakeLighting > 0.5)
        {
            brightness = 1.0;
        }
        
        vec3 lightDirection = -unitLightVector;
        vec3 unitVectorToCamera = normalize(toCameraVector);
        vec3 reflectedLightDirection = reflect(lightDirection, unitNormal);
        
        float specularFactor = max(dot(reflectedLightDirection, unitVectorToCamera), 0.0);
        float dampedFactor = pow(specularFactor, 20.0); //shineDamper
        
        totalDiffuse  = totalDiffuse  + (               brightness*lightColor[i])/attFactor;
        totalSpecular = totalSpecular + (dampedFactor*reflectivity*lightColor[i])/attFactor;
    }
    
    if (glowAmount > 0.0)
    {
        totalDiffuse = vec3(glowAmount);
    }
    
    totalDiffuse = max(totalDiffuse, 0.32); //ambient light
    
    out_Color = vec4(totalDiffuse, 1.0)*rawTextureColor + vec4(totalSpecular, rawTextureColor.a);
}
