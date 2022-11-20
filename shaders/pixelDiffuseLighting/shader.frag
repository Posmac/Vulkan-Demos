#version 450

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;

layout(push_constant) uniform LightParameters {
	vec3 position;
} Light;

void main()
{
	const vec3 norm = normalize(vertNormal);
	const vec3 lightDir = normalize(Light.position - vertPosition);

	const float ambientFactor = 0.2;
	const vec4 ambientColor = vec4(0.1, 0.2, 0.0, 1.0);
	const vec4 ambientComponent = ambientFactor * ambientColor;

	const float diffuseFactor = max(0.0, dot(norm, lightDir));

	const vec3 halfVector = normalize(- vertPosition + lightDir);
	const float shineness = 64;
	float specular = pow(dot(halfVector, norm), shineness);

	FragColor = ambientComponent + vec4(vec3(diffuseFactor), 1.0) + vec4(vec3(specular), 1.0);
}