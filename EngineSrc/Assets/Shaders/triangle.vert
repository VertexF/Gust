#version 450

layout (location = 0) out vec3 outColour;

void main()
{
    const vec3 positions[3] = vec3[3]
    (
        vec3(1.f, 1.f, 0.f),
        vec3(-1.f, 1.f, 0.f),
        vec3(0.f, -1.f, 0.f)
    );

    const vec3 colour[3] = vec3[3]
    (
        vec3(1.f, 0.f, 0.f),
        vec3(0.f, 1.f, 0.f),
        vec3(0.f, 0.f, 1.f)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 1.f);
    outColour = colour[gl_VertexIndex];
}