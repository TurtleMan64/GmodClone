#version 330

in vec2 pass_textureCoords;

out vec4 out_color;

uniform vec3 color;
uniform float alpha;
uniform sampler2D fontAtlas;
uniform float fontHeight;

const float width = 0.45;
const float edge = 0.2;


float smoothlyStep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

void main(void)
{
	float dist = 1.0 - texture(fontAtlas, pass_textureCoords).a;
	float edgeAlpha = 1.0 - smoothlyStep(width, width + edge/(fontHeight*60), dist);
	
	out_color = vec4(color, alpha*edgeAlpha);
}
