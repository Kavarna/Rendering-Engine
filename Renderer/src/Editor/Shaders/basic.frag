#version 450

layout(push_constant) uniform Color
{
    vec4 color;
} ubo;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = ubo.color;
}