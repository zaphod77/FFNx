#version 400

layout(std140) uniform Globals
{
    mat4 viewport;
    mat4 d3dViewport;
    mat4 d3dProjection;
    mat4 Flags;
};

// Vertex input from app
in vec4 a_position;
in vec4 a_color;
in vec2 a_texcoord0;

// Vertex output from the vertex shader
out vec4 v_position;
out vec4 v_color;
out vec2 v_texcoord0;

#define isTLVertex Flags[0].x > 0.0
#define blendMode Flags[0].y
#define isFBTexture Flags[0].z > 0.0

void main()
{
	vec4 pos = a_position;
    vec4 color = a_color;
    vec2 coords = a_texcoord0;

    color.rgba = color.bgra;

    if (isTLVertex)
    {
        pos.w = 1.0 / pos.w;
        pos.xyz *= pos.w;
        pos = viewport * pos;
    }
    else
    {
        pos = d3dViewport * pos;
        pos = d3dProjection * pos;
        pos = viewport * pos;

        if (color.a > 0.5) color.a = 0.5;
    }

    if (blendMode == 4.0) color.a = 1.0;
    else if (blendMode == 3.0) color.a = 0.25;

    if (isFBTexture) coords.t = 1.0 - coords.t;

    v_position = pos;
    v_color = color;
    v_texcoord0 = coords;
}
