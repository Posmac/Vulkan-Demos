#version 450

layout(location = 0) in float vertColor;
layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(vertColor);
}