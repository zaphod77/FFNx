$input v_color0, v_texcoord0

#include <bgfx/bgfx_shader.sh>

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

SAMPLER2D(tex, 0);
SAMPLER2D(tex_u, 1);
SAMPLER2D(tex_v, 2);

uniform mat4 FSFlags;

#define isFullRange FSFlags[0].x > 0.0
#define isTexture FSFlags[0].y > 0.0
#define isYUV FSFlags[0].z > 0.0
#define isFBTexture FSFlags[0].w > 0.0
// ---
#define inAlphaRef FSFlags[1].x
#define inAlphaFunc FSFlags[1].y
#define doAlphaTest FSFlags[1].z > 0.0
#define inheritTextureAlpha FSFlags[1].w > 0.0

void main()
{
	vec4 color = v_color0;
    
    if (isYUV)
    {
        vec4 yuv = vec4(
            texture2D(tex, v_texcoord0.st).x - 0.0625,
            texture2D(tex_u, v_texcoord0.st).x - 0.5,
            texture2D(tex_v, v_texcoord0.st).x - 0.5,
            1.0
        );

        if (isFullRange) color = mul(jpeg_rgb_transform, yuv);
        else color = mul(mpeg_rgb_transform, yuv);

        color.a = 1.0f;
    }
    else
    {
        if (doAlphaTest)
        {
            // ALPHA TEST
            if ( inAlphaFunc == 0.0) //NEVER
                discard;
            else if ( inAlphaFunc == 1.0) //LESS
                if (!(color.a < inAlphaRef)) discard;
            else if ( inAlphaFunc == 2.0) //EQUAL
                if (!(color.a == inAlphaRef)) discard;
            else if ( inAlphaFunc == 3.0) //LEQUAL
                if (!(color.a <= inAlphaRef)) discard;
            else if ( inAlphaFunc == 4.0) //GREATER
                if (!(color.a > inAlphaRef)) discard;
            else if ( inAlphaFunc == 5.0) //NOTEQUAL
                if (!(color.a != inAlphaRef)) discard;
            else if ( inAlphaFunc == 6.0) //GEQUAL
                if (!(color.a >= inAlphaRef)) discard;
        }

        vec4 texture_color = texture2D(tex, v_texcoord0.st);

        if (isTexture)
        {
            if(isFBTexture && texture_color.rgb == vec3(0.0, 0.0, 0.0)) discard;

            if (texture_color.a == 0.0) discard;

            color *= texture_color;

            if(inheritTextureAlpha) color.a = texture_color.a;
        }
    }

	gl_FragColor = color;
}
