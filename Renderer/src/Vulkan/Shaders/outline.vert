#version 450

struct PerObjectInfo {
	mat4 world;
    uint materialIndex;
};

layout(push_constant) uniform ObjectIndex
{
    uint objectIndex;
    float outlineWidth;
    vec4 outlineColor;
} PushConstant;

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer {

	PerObjectInfo objects[];
} objectBuffer;

layout(std140, set = 0, binding = 1) uniform UniformBufferObject
{
    mat4 viewProj;
} uniformObject;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inNormal;

void main()
{
    PerObjectInfo ob = objectBuffer.objects[PushConstant.objectIndex];
    
    vec3 pos = position;
    pos += inNormal * PushConstant.outlineWidth;
    gl_Position = uniformObject.viewProj * ob.world * vec4(pos, 1.0);
}