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

    pos.w = 1.0 / pos.w;
    pos.xyz *= pos.w;
    pos = mul(g_Constants.orthoProjection, pos);

    OUT.position = pos;
    OUT.color = color;
    OUT.coords = coords;
}

