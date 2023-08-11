#version 400 core

in vec3 toCameraVector;
in vec3 toLightVector[1];
in vec2 pass_textureCoords;
in vec4 pass_vertexColor;
//in vec4 pass_worldCoords; //only needed for light map
in vec2 pass_textureCoordsLightmap;

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

uniform sampler2D lightMap;

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
	
	vec4 diffuse = mix(texture(textureSampler, pass_textureCoords), texture(textureSampler2, pass_textureCoords), mixFactor);
	vec4 light = texture(lightMap, pass_textureCoordsLightmap);
	vec4 rawTextureColor = diffuse*light;
	rawTextureColor.a = rawTextureColor.a*0.5; //for some bizarre reason, 0.5 alpha is seen as full opaque...

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
    
    vec3 totalSpecular = vec3(0);
    
    for (int i = 0; i < 1; i++)
    {
        float dist = length(toLightVector[i]); //toLightVector is making things dark in eq. if the cam direction has 0 for y, it happens
        float attFactor = attenuation[i].x + (attenuation[i].y * dist) + (attenuation[i].z * dist * dist);
        
        vec3 unitLightVector = normalize(toLightVector[i]);
        
        float nDotl = dot(unitNormal, unitLightVector);
        
        vec3 lightDirection = -unitLightVector;
        vec3 unitVectorToCamera = normalize(toCameraVector);
        vec3 reflectedLightDirection = reflect(lightDirection, unitNormal);
        
        float specularFactor = max(dot(reflectedLightDirection, unitVectorToCamera), 0.0);
        float dampedFactor = pow(specularFactor, 20.0); //shineDamper
        
        totalSpecular = totalSpecular + ((dampedFactor*reflectivity*lightColor[i])/attFactor)*((light.r + light.g + light.b)*0.333333);
    }

    out_Color = rawTextureColor + vec4(totalSpecular, rawTextureColor.a);
}
