#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TextCoord;

void main ()
{
	gl_Position = position;
	v_TextCoord = texCoord;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TextCoord;

uniform vec4 u_color;
uniform sampler2D u_Texture;

void main ()
{
    vec2 flippedUV = vec2(v_TextCoord.x, 1.0 - v_TextCoord.y);
    vec4 texColor = texture(u_Texture, flippedUV);
    color = texColor;
};
