#version 400 core

in vec3 toCameraVector;
in vec3 toLightVector[4];
in vec2 pass_textureCoords;
in vec4 pass_vertexColor;
in vec4 pass_worldCoords; //only needed for light map

out vec4 out_Color;

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;
uniform sampler2D normalMap;
uniform vec3  lightColor[4];
uniform vec3  attenuation[4];
uniform float reflectivity;
uniform float useFakeLighting;
uniform vec3  skyColor;
uniform int   hasTransparency;
uniform float glowAmount;
uniform vec3  baseColor;
uniform float baseAlpha;
uniform float mixFactor;

// shadow map
uniform sampler2D lightMap;
uniform float lightMapOriginX;
uniform float lightMapOriginY;
uniform float lightMapOriginZ;
uniform float lightMapSizeX;
uniform float lightMapSizeY;
uniform float lightMapSizeZ;

uniform float fogDensity;
uniform float fogGradient;
uniform float fogScale; //per material
uniform float fogBottomPosition;
uniform float fogBottomThickness;

// Stuff for random noise
uniform uint clock;
uniform float noise;
uniform uint entityId;


// Random function found here
// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// https://stackoverflow.com/a/17479300/17304333

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random(uint x)
{
    return floatConstruct(hash(x));
}

void main(void)
{
    if (noise < 1.0)
    {
        uint fragX = uint(gl_FragCoord.x);
        uint fragY = uint(gl_FragCoord.y);
        
        uint idx = fragX + fragY*2000 + (clock + entityId)*2073600;
        
        float ran = random(idx);
        
        if (ran > noise)
        {
            discard;
        }
    }
    
    vec4 rawTextureColor = mix(texture(textureSampler, pass_textureCoords), texture(textureSampler2, pass_textureCoords), mixFactor);
    rawTextureColor.rgb *= baseColor*pass_vertexColor.rgb;
    rawTextureColor.a   *= baseAlpha*pass_vertexColor.a*0.5; //for some bizarre reason, 0.5 alpha is seen as full opaque...
    
    //const float lightMapOriginX = -28.690516;
    //const float lightMapOriginY = -10.288731;
    //const float lightMapOriginZ = -28.690516;
    //const float lightMapSizeX   =  57.381031;
    //const float lightMapSizeY   =  22.145657;
    //const float lightMapSizeZ   =  57.381031;
    
    // don't do shadow map stuff for some stages. we can tell if a stage has it if this value is too small
    if (lightMapSizeX > 2.0)
    {
        //uint fragX = uint(gl_FragCoord.x);
        //uint fragY = uint(gl_FragCoord.y);
        //
        //uint idx = fragX + fragY*2000 + (clock + entityId)*2073600;
        
        //float ran1 = random(idx);
        //float ran2 = random(idx+1);
        //
        //float dx = (ran2/128)*cos(ran1*3.14*2);
        //float dz = (ran2/128)*sin(ran1*3.14*2);
        
        //float dx = random(idx)  /2000.0;
        //float dy = random(idx+1)/2000.0;
        //float dz = random(idx+2)/2000.0;
        
        //vec4 lightMapColor = texture(lightMap, 
        //vec3( ((pass_worldCoords.x - lightMapOriginX)/lightMapSizeX) + dx,
        //     -((pass_worldCoords.y - lightMapOriginY)/lightMapSizeY) + dy,
        //      ((pass_worldCoords.z - lightMapOriginZ)/lightMapSizeZ) + dz));
        //
        //const float ambientLight = 0.1;
        //
        //rawTextureColor.rgb *= min(lightMapColor.r + ambientLight, 1.0);
        
        vec2 ourSpot = vec2(((pass_worldCoords.x) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z) - lightMapOriginZ)/lightMapSizeZ);
        
        if (ourSpot.x >= 0.0 && ourSpot.x < 1.0 && ourSpot.y >= 0.0 && ourSpot.y < 1.0)
        {
            //todo these hard coded values need to be adjusted basedon the map size
            vec2 ourSpot1 = vec2(((pass_worldCoords.x - 0.05) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z - 0.05) - lightMapOriginZ)/lightMapSizeZ);
            vec2 ourSpot2 = vec2(((pass_worldCoords.x + 0.05) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z - 0.05) - lightMapOriginZ)/lightMapSizeZ);
            vec2 ourSpot3 = vec2(((pass_worldCoords.x - 0.05) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z + 0.05) - lightMapOriginZ)/lightMapSizeZ);
            vec2 ourSpot4 = vec2(((pass_worldCoords.x + 0.05) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z + 0.05) - lightMapOriginZ)/lightMapSizeZ);
            
            float depth1 = texture(lightMap, ourSpot1).r;
            float depth2 = texture(lightMap, ourSpot2).r;
            float depth3 = texture(lightMap, ourSpot3).r;
            float depth4 = texture(lightMap, ourSpot4).r;
            
            if ((pass_worldCoords.y < ((lightMapOriginY + lightMapSizeY) - (depth1*lightMapSizeY)) - 0.03) &&
                (pass_worldCoords.y < ((lightMapOriginY + lightMapSizeY) - (depth2*lightMapSizeY)) - 0.03) &&
                (pass_worldCoords.y < ((lightMapOriginY + lightMapSizeY) - (depth3*lightMapSizeY)) - 0.03) &&
                (pass_worldCoords.y < ((lightMapOriginY + lightMapSizeY) - (depth4*lightMapSizeY)) - 0.03))
            {
                uint fragX = uint(gl_FragCoord.x);
                uint fragY = uint(gl_FragCoord.y);
                
                uint idx = fragX + fragY*2000 + (clock + entityId)*2073600;
                
                float ran1 = random(idx);
                float ran2 = random(idx+1);
                
                int numInside = 4;
                
                for (int i = 0; i < 4; i++)
                {
                    float dx = ((ran2 * 0.2) + 0.05)*cos((ran1 + (i / 4.0))* 3.14 * 2);
                    float dz = ((ran2 * 0.2) + 0.05)*sin((ran1 + (i / 4.0))* 3.14 * 2);
                    
                    vec2 ranSpot = vec2(((pass_worldCoords.x + dx) - lightMapOriginX)/lightMapSizeX, ((pass_worldCoords.z + dz) - lightMapOriginZ)/lightMapSizeZ);
                    
                    float ranDepth = texture(lightMap, ranSpot).r;
                    
                    if (pass_worldCoords.y < ((lightMapOriginY + lightMapSizeY) - (ranDepth*lightMapSizeY)) - 0.03)
                    {

                    }
                    else
                    {
                        numInside = 0;
                        break;
                    }
                }
                
                if (numInside == 4)
                {
                    rawTextureColor.rgb *= 0.75;
                }
            }
        }
    }

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
