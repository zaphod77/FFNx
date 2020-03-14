#include "shaders/FFNx.fxh"

#define isTLVertex g_Constants.VSFlags.x > 0.0
#define blendMode g_Constants.VSFlags.y
#define isFBTexture g_Constants.VSFlags.z > 0.0

void main(in VSIn IN, out PSIn OUT)
{
	float4 pos = IN.position;
    float4 color = IN.color;
    float2 coords = IN.coords;

    color.rgba = color.bgra;

    if (isTLVertex)
    {
        pos.w = 1.0 / pos.w;
        pos.xyz *= pos.w;
        pos = mul(g_Constants.orthoProjection, pos);
    }
    else
    {
        pos = mul(mul(g_Constants.d3dViewport,mul(g_Constants.d3dProjection, g_Constants.worldView)), float4(pos.xyz, 1.0));

        if (color.a > 0.5) color.a = 0.5;
    }

    if (blendMode == 4.0) color.a = 1.0;
    else if (blendMode == 3.0) color.a = 0.25;

    if (isFBTexture) coords.y = 1.0 - coords.y;

    OUT.position = pos;
    OUT.color = color;
    OUT.coords = coords;
}

