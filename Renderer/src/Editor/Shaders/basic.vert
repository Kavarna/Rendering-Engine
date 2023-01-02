#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 world;
} ubo;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = ubo.world * vec4(position, 1.0);
}