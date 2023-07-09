#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 fragColor;

layout(push_constant, std430) uniform PushConstants {
    mat4 modelViewProjection;
} pushConstants;

void main()
{
    gl_Position = pushConstants.modelViewProjection * position;
    fragColor = color;
}