#version 450

layout (location = 0) in vec3 inColour;

layout (location = 0) out vec4 outColour;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    outColour = vec4(inColour, 1.0);
}