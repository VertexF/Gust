#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColour;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColour;

void main()
{
    outColour = texture(texSampler, fragTexCoord) * vec4(fragColour, 1.0);
    //outColour = texture(texSampler, fragTexCoord);
    //outColour = vec4(fragColour * texture(texSampler, fragTexCoord).rgb, 1.0);
}
