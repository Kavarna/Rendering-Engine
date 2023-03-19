#version 450


layout(std140, set = 0, binding = 1) uniform UniformBufferObject
{
    mat4 viewProj;
} uniformObject;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{    
    gl_Position = uniformObject.viewProj * vec4(position, 1.0);
    outColor = inColor;
}