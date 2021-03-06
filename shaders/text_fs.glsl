#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;
uniform vec4 highlightColor;

void main()
{
	if (highlightColor.a > 0.0f) {
		color = highlightColor;
	}
	else {
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
		if (sampled.a < 0.5)
			discard;
		color = textColor * sampled;
	}
}