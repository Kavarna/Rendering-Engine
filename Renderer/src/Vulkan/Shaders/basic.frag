#version 450

struct Material
{
    vec4 color;
};

layout(std140, set = 0, binding = 2) readonly buffer MaterialBuffer {

	Material materials[];
} materialBuffer;

layout(std140, set = 0, binding = 3) uniform UniformBufferObject
{
    vec4 color;
    vec3 direction;
} uniformObject;

layout(location = 0) in flat uint materialIndex;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 normal = normalize(inNormal);
    vec4 color = materialBuffer.materials[materialIndex].color;
    float factor = max(dot(normal, uniformObject.direction), 0.0f);

    outColor = color * factor;
}