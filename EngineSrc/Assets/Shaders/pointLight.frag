#version 450

layout (location = 0) in vec2 inOffset;
layout (location = 0) out vec4 outColour;

struct PointLight
{
    vec4 position;
    vec4 colour;
};

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColour;
    PointLight pointLights[10];
    int numLights;
} UBO;

layout(push_constant) uniform Push
{
    vec4 position;
    vec4 colour;
    float radius;
} push;

const float M_PI = 3.1415926538;

void main()
{
    float distance = sqrt(dot(inOffset, inOffset));
    if(distance >= 1.0)
    {
        discard;
    }

    float cosDistance = 0.5 * (cos(distance * M_PI) + 1.0);
    outColour = vec4(push.colour.xyz + 0.5 * cosDistance, cosDistance);
}
