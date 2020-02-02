#version 400

layout(std140) uniform Globals
{
    mat4 viewport;
    mat4 d3dViewport;
    mat4 d3dProjection;
    mat4 Flags;
};

uniform sampler2D tex;
uniform sampler2D tex_u;
uniform sampler2D tex_v;

// Fragment input from the vertex shader
in vec4 v_position;
in vec4 v_color;
in vec2 v_texcoord0;

// Fragment output color
out vec4 o_color;

const mat4 mpeg_rgb_transform = mat4(
    +1.164, +1.164, +1.164, +0.000,
    +0.000, -0.392, +2.017, +0.000,
    +1.596, -0.813, +0.000, +0.000,
    +0.000, +0.000, +0.000, +1.000
);

const mat4 jpeg_rgb_transform = mat4(
    +1.000, +1.000, +1.000, +0.000,
    +0.000, -0.343, +1.765, +0.000,
    +1.400, -0.711, +0.000, +0.000,
    +0.000, +0.000, +0.000, +1.000
);

#define isFBTexture Flags[0].z > 0.0
// ---
#define isFullRange Flags[1].x > 0.0
#define isTexture Flags[1].y > 0.0
#define isYUV Flags[1].z > 0.0
#define inheritTextureAlpha Flags[1].w > 0.0
// ---
#define inAlphaRef Flags[2].x
#define inAlphaFunc Flags[2].y
#define doAlphaTest Flags[2].z > 0.0

void main()
{
    vec4 color = v_color;

    if (isYUV)
    {
        vec4 yuv = vec4(
            texture(tex, v_texcoord0).x - (1.0 / 16.0),
            texture(tex_u, v_texcoord0).x - 0.5,
            texture(tex_v, v_texcoord0).x - 0.5,
            1.0
        );

        if (isFullRange) color = jpeg_rgb_transform * yuv;
        else color = mpeg_rgb_transform * yuv;

        color.a = 1.0f;
    }
    else
    {
        if (doAlphaTest)
        {
            // ALPHA TEST
            if (inAlphaFunc == 0.0) //NEVER
                discard;
            else if (inAlphaFunc == 1.0) //LESS
                if (!(color.a < inAlphaRef)) discard;
            else if (inAlphaFunc == 2.0) //EQUAL
                if (!(color.a == inAlphaRef)) discard;
            else if (inAlphaFunc == 3.0) //LEQUAL
                if (!(color.a <= inAlphaRef)) discard;
            else if (inAlphaFunc == 4.0) //GREATER
                if (!(color.a > inAlphaRef)) discard;
            else if (inAlphaFunc == 5.0) //NOTEQUAL
                if (!(color.a != inAlphaRef)) discard;
            else if (inAlphaFunc == 6.0) //GEQUAL
                if (!(color.a >= inAlphaRef)) discard;
        }

        vec4 texture_color = texture(tex, v_texcoord0);

        if (isTexture)
        {
            if (isFBTexture && texture_color.rgb == vec3(0.0, 0.0, 0.0)) discard;

            if (texture_color.a == 0.0) discard;

            color *= texture_color;

            if (inheritTextureAlpha) color.a = texture_color.a;
        }
    }

    o_color = color;
}
