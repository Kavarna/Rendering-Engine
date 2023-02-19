#version 450

struct Material
{
    vec4 color;
};

layout(std140, set = 0, binding = 2) readonly buffer MaterialBuffer {

	Material materials[];
} materialBuffer;

layout(location = 0) in flat uint materialIndex;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = materialBuffer.materials[materialIndex].color;
}