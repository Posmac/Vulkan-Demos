#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform UniformBuffer
{
	 mat4 ModelMatrix;
     mat4 ViewMatrix;
     mat4 ProjectionMatrix;
} ubo;
 
 layout(location = 0) out vec3 vertexPosition;
 layout(location = 1) out vec3 vertexNormal;

void main()
{
	mat4 modelViewMatrix = ubo.ViewMatrix * ubo.ModelMatrix;
	vertexNormal = mat3(modelViewMatrix) * inNormal;
	vec4 viewVertexPosition = modelViewMatrix * vec4(inPos, 1.0);
	vertexPosition = viewVertexPosition.xyz;
	gl_Position = ubo.ProjectionMatrix * viewVertexPosition;

}