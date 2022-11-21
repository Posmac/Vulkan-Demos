#version 450

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextCoords;
layout(location = 2) in vec3 TangentLightPos;
layout(location = 3) in vec3 TangentViewPos;
layout(location = 4) in vec3 TangentFragPos;

layout(set = 0, binding = 1) uniform sampler2D normalMap;

void main()
{
	vec3 normal = texture(normalMap, TextCoords).xyz;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 color = vec3(0.4, 0.3, 0.2);
	vec3 ambient = color * 0.1;
	vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
	float diff = max(0.0, dot(lightDir, normal));
	vec3 diffuse = diff * color;

	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfVec = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfVec), 0.0), 32.0);
	vec3 specular = color * spec;

	FragColor = vec4(ambient + diffuse + specular, 1.0);
}