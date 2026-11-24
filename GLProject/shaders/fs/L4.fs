#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform bool uUseSolidColor;
uniform vec3 uSolidColor;
uniform vec3 uTintColor;

void main()
{
    if (uUseSolidColor)
    {
        FragColor = vec4(uSolidColor, 1.0);
    }
    else
    {
        FragColor = texture(texture1, TexCoord) * vec4(uTintColor, 1.0);
    }
}