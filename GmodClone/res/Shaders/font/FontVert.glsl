#version 330

in vec2 position;
in vec2 textureCoords;

out vec2 pass_textureCoords;

uniform vec2 translation;
uniform float screenRatio;
uniform float fontHeight;

void main(void)
{
	gl_Position = vec4((2*(position.x*screenRatio*fontHeight) - 1) + 2*translation.x, (2*(-position.y*fontHeight) + 1) - 2*translation.y, 0.0, 1.0);
	pass_textureCoords = textureCoords;
}
