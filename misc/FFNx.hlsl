struct InputVS
{
    float4 a_position : POSITION;
    float4 a_color : COLOR;
    float2 a_texcoord0 : TEXCOORD0;
};

struct OutputVS
{
    float4 v_position : POSITION;
    float4 v_color : COLOR;
    float2 v_texcoord0 : TEXCOORD0;
};

float4x4 viewportMatrix;
float4x4 d3dViewport;
float4x4 d3dProjection;
float4x4 Flags;

#define isTLVertex Flags[0].x > 0.0
#define blendMode Flags[0].y
#define isFBTexture Flags[0].z > 0.0

OutputVS VSMain(InputVS inVS)
{
    OutputVS ret;

	float4 pos = inVS.a_position;
    float4 color = inVS.a_color;
    float2 coords = inVS.a_texcoord0;

    color.rgba = color.bgra;

    if (isTLVertex)
    {
        pos.w = 1.0 / pos.w;
        pos.xyz *= pos.w;
        pos = mul(viewportMatrix, pos);
    }
    else
    {
        pos = mul(d3dViewport, pos);
        pos = mul(d3dProjection, pos);
        pos = mul(viewportMatrix, pos);

        if (color.a > 0.5) color.a = 0.5;
    }

    if (blendMode == 4.0) color.a = 1.0;
    else if (blendMode == 3.0) color.a = 0.25;

    if (isFBTexture) coords.t = 1.0 - coords.t;

    ret.v_position = pos;
    ret.v_color = color;
    ret.v_texcoord0 = coords;

    return ret;
}

const float4x4 mpeg_rgb_transform = {
    +1.164, +1.164, +1.164, +0.000,
    +0.000, -0.392, +2.017, +0.000,
    +1.596, -0.813, +0.000, +0.000,
    +0.000, +0.000, +0.000, +1.000
};

const float4x4 jpeg_rgb_transform = {
    +1.000, +1.000, +1.000, +0.000,
    +0.000, -0.343, +1.765, +0.000,
    +1.400, -0.711, +0.000, +0.000,
    +0.000, +0.000, +0.000, +1.000
};

Texture2D tex;
SamplerState texSampler;
Texture2D tex_u;
SamplerState tex_uSampler;
Texture2D tex_v;
SamplerState tex_vSampler;

#define isFullRange Flags[1].x > 0.0
#define isTexture Flags[1].y > 0.0
#define isYUV Flags[1].z > 0.0
#define inheritTextureAlpha Flags[1].w > 0.0
// ---
#define inAlphaRef Flags[2].x
#define inAlphaFunc Flags[2].y
#define doAlphaTest Flags[2].z > 0.0

float4 FSMain(OutputVS outVS) : SV_Target
{
    float4 color = outVS.v_color;

    if (isYUV)
    {
        float4 yuv = float4(
            tex.Sample(texSampler, outVS.v_texcoord0.st).x - (1.0 / 16.0),
            tex_u.Sample(tex_uSampler, outVS.v_texcoord0.st).x - 0.5,
            tex_v.Sample(tex_vSampler, outVS.v_texcoord0.st).x - 0.5,
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

        float4 texture_color = tex.Sample(texSampler, outVS.v_texcoord0.st);

        if (isTexture)
        {
            if (isFBTexture && all(texture_color.rgb == float3(0.0, 0.0, 0.0))) discard;

            if (texture_color.a == 0.0) discard;

            color *= texture_color;

            if (inheritTextureAlpha) color.a = texture_color.a;
        }
    }

    return color;
}
