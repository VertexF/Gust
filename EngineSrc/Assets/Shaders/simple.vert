#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inUV;

layout (location = 0) out vec3 outColour;

layout (set = 0, binding = 0) uniform GlobalUBO
{
    mat4 projectionViewMatrix;
    vec4 ambientLightColour;
    vec3 lightPosition;
    vec4 lightColour;
} UBO;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = UBO.projectionViewMatrix * positionWorld;

    vec4 normalWorldSpace = normalize(push.normalMatrix * vec4(inNormal, 0.0));
    
    vec4 directionToLight = vec4(UBO.lightPosition, 1.0) - positionWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);

    vec3 lightColour = UBO.lightColour.xyz * UBO.lightColour.w * attenuation;
    vec3 ambientLight = UBO.ambientLightColour.xyz * UBO.ambientLightColour.w;
    vec3 diffuseLight = lightColour * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

    outColour = (diffuseLight + ambientLight) * inColour;
}