#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 world;
} ubo;


layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = ubo.world * vec4(position, 1.0);
    fragColor = vec3(1.0, 0.0, 0.0);
}