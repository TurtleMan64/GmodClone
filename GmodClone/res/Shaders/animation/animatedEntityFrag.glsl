#version 150

const vec2 lightBias = vec2(0.7, 0.6);//just indicates the balance between diffuse and ambient lighting

in vec2 pass_textureCoords;
in vec3 pass_normal;
in vec4 pass_worldPosition;

out vec4 out_colour;

uniform sampler2D diffuseMap;
uniform vec3 lightDirection;

// shadow map
uniform sampler2D lightMap;
uniform float lightMapOriginX;
uniform float lightMapOriginY;
uniform float lightMapOriginZ;
uniform float lightMapSizeX;
uniform float lightMapSizeY;
uniform float lightMapSizeZ;

void main(void)
{
    vec4 diffuseColor = texture(diffuseMap, pass_textureCoords);
    
    if (diffuseColor.a < 0.02)
    {
        discard;
    }
    //diffuseColor.a = 1.0;
    
    //{
    //    vec4 lightMapColor = texture(lightMap, 
    //      vec3( ((pass_worldPosition.x - lightMapOriginX)/lightMapSizeX),
    //           -((pass_worldPosition.y - lightMapOriginY)/lightMapSizeY),
    //            ((pass_worldPosition.z - lightMapOriginZ)/lightMapSizeZ)));
    //    
    //    
    //    const float ambientLight = 0.1;
    //    
    //    
    //    diffuseColor.rgb *= min(lightMapColor.r + ambientLight, 1.0);
    //}
    
    // don't do shadow map stuff for some stages. we can tell if a stage has it if this value is too small
    if (lightMapSizeX > 2.0)
    {
        vec2 ourSpot = vec2(((pass_worldPosition.x) - lightMapOriginX)/lightMapSizeX, ((pass_worldPosition.z) - lightMapOriginZ)/lightMapSizeZ);
        
        if (ourSpot.x >= 0.0 && ourSpot.x < 1.0 && ourSpot.y >= 0.0 && ourSpot.y < 1.0)
        {
            float depth = texture(lightMap, ourSpot).r;
            
            if (pass_worldPosition.y < ((lightMapOriginY + lightMapSizeY) - (depth*lightMapSizeY)) - 0.03)
            {
                diffuseColor.rgb *= 0.75;
            }
        }
    }
    
	vec3 unitNormal = normalize(pass_normal);
	float diffuseLight = max(dot(-lightDirection, unitNormal), 0.0) * lightBias.x + lightBias.y;
	out_colour = diffuseColor * diffuseLight;
}
