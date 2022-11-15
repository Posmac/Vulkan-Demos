#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	mat4 MVMatrix;
	mat4 PMatrix;
};

layout(location = 0) out float vertColor;

void main()
{
	gl_Position = PMatrix * MVMatrix * inPos;
	vec3 normal = mat3(MVMatrix) * inNormal;

	vertColor = max(0.0, dot(normal, vec3(0.58, 0.58, 0.58))) + 0.1;
}