#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec3 Color;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main() {
	vs_out.FragPos = aPos;
	vs_out.Normal = aNormal;
	vs_out.Color = aColor;

	gl_Position = projection * view * vec4(aPos, 1.0);
}
