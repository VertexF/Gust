#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColour;

layout (location = 0) out vec3 outColour;

layout(pushContant) uniform constants
{
    vec4 data;
    mat4 modelViewPerspective;
} PushConstants;

void main()
{
    gl_Position = PushConstants.modelViewPerspective * vec4(inPosition, 1.f);
    outColour = inColour;
}