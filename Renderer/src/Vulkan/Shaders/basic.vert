#version 450

struct PerObjectInfo {
	mat4 world;
    uint materialIndex;
};

layout(push_constant) uniform ObjectIndex
{
    uint objectIndex;
} PushConstant;

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer {

	PerObjectInfo objects[];
} objectBuffer;

layout(set = 0, binding = 1) uniform UniformBufferObject
{
    mat4 viewProj;
} uniformObject;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = uniformObject.viewProj * objectBuffer.objects[PushConstant.objectIndex].world * vec4(position, 1.0);
}