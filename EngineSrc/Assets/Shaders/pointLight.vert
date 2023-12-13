#version 450

const vec2 OFFSETS[6] = vec2[]
(
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

layout (location = 0) out vec2 outOffset;

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

const float LIGHT_RADIUS = 0.05;

void main()
{
    outOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = { UBO.view[0][0], UBO.view[1][0], UBO.view[2][0] };
    vec3 cameraUpWorld = { UBO.view[0][1], UBO.view[1][1], UBO.view[2][1] };

    vec3 positionWorld = push.position.xyz +
                         push.radius * outOffset.x * cameraRightWorld + 
                         push.radius * outOffset.y * cameraUpWorld;

    gl_Position = UBO.projection * (UBO.view * vec4(positionWorld, 1.0));
}