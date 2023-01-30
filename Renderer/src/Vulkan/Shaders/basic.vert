#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 worldViewProjection;
} ubo;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = ubo.worldViewProjection * vec4(position, 1.0);
}