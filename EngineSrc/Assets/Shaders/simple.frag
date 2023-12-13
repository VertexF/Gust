#version 450

layout (location = 0) in vec3 inColour;
layout (location = 1) in vec4 inPosWorld;
layout (location = 2) in vec4 inNormalWorld;

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
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec3 diffuseLight = UBO.ambientLightColour.xyz * UBO.ambientLightColour.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(inNormalWorld.xyz);

    vec3 cameraPosWorld = UBO.inverseView[3].xyz;
    //This gets the light so it's directed to the users.
    vec3 viewDirection = normalize(cameraPosWorld - inPosWorld.xyz);

    for (int i = 0; i < UBO.numLights; i++)
    {
        PointLight light = UBO.pointLights[i];
        vec3 directionToLight = (light.position - inPosWorld).xyz;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);

        directionToLight = normalize(directionToLight);

        float cosOfIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.colour.xyz * light.colour.w * attenuation;

        diffuseLight += intensity * cosOfIncidence;

        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512.0);

        specularLight += intensity * blinnTerm;
    }

    outColour = vec4(diffuseLight * inColour + specularLight * inColour, 1.0);
}