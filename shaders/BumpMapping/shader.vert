#version 450

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTextCoord;
layout(location = 3) in vec3 vertTangent;
layout(location = 4) in vec3 vertBitangent;

layout(set = 0, binding = 0) uniform UniformBuffer {
	mat4 Model;
	mat4 View;
	mat4 Projection;
};

layout(push_constant) uniform SceneConstants 
{
	vec4 LightPos;
	vec4 ViewPos;
} Scene;

layout(location = 0) out vec3 Position;
layout(location = 1) out vec2 TextCoords;
layout(location = 2) out vec3 TangentLightPos;
layout(location = 3) out vec3 TangentViewPos;
layout(location = 4) out vec3 TangentFragPos;

void main()
{
	Position = vec3(Model * vec4(vertPosition, 1.0));
	TextCoords = vertTextCoord;

	mat3 normalMatrix = transpose(inverse(mat3(Model)));
	vec3 T = normalize(normalMatrix * vertTangent);
	vec3 N = normalize(normalMatrix * vertNormal);
	T = normalize(T - dot(T,N) * N);
	vec3 B = cross(N,T);

	mat3 TBN = transpose(mat3(T, B, N));
	TangentLightPos = TBN * Scene.LightPos.xyz;
	TangentViewPos = TBN * Scene.ViewPos.xyz;
	TangentFragPos = TBN * Position;

	gl_Position = Projection * View * Model * vec4(vertPosition, 1.0);
}