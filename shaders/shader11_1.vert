#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	 mat4 ModelMatrix;
     mat4 ViewMatrix;
     mat4 ProjectionMatrix;
} ubo;

layout(location = 0) out float vertColor;

void main()
{
	gl_Position = ubo.ProjectionMatrix * ubo.ViewMatrix * ubo.ModelMatrix * vec4(inPos, 1.0);
	vec3 normal = mat3(ubo.ModelMatrix * ubo.ViewMatrix) * inNormal;

	vertColor = max(0.0, dot(normal, vec3(0.58, 0.58, 0.58))) + 0.1;
}