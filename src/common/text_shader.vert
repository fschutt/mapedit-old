R"(

#version 130
in vec4 TexturePlusUV;
out vec2 TexCoords;

uniform mat4 projection;
uniform float rotation;

void main()
{
    gl_Position = projection * vec4(TexturePlusUV.xy, 0.0, 1.0);
    TexCoords = TexturePlusUV.zw;
}

)"
