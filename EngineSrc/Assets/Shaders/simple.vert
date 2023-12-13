#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inUV;

layout (location = 0) out vec3 outColour;
layout (location = 1) out vec4 outPosWorld;
layout (location = 2) out vec4 outNormalWorld;

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
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = UBO.projection * (UBO.view * positionWorld);
    outNormalWorld = normalize(push.normalMatrix * vec4(inNormal, 0.0));
    outPosWorld = positionWorld;
    outColour = inColour;
}