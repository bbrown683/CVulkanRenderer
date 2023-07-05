#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragColor;

layout(location = 0) out vec4 fragmentColor;

layout(binding = 0) uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, fragTexCoord);
    fragmentColor = fragColor * texColor;
}