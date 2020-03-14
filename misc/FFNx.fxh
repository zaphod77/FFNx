struct Constants
{
    float4 VSFlags;
    float4 FSAlphaFlags;
    float4 FSMiscFlags;
    float4x4 orthoProjection;
    float4x4 d3dViewport;
    float4x4 d3dProjection;
    float4x4 worldView;
};

cbuffer SConstants
{
    Constants g_Constants;
};

struct VSIn
{
    float4 position : ATTRIB0;
    float4 color    : ATTRIB1;
    float2 coords   : ATTRIB2;
};

struct PSIn
{
    float4 position  : SV_Position;
    float4 color     : COLOR;
    float2 coords    : TEX_COORD;
};

struct PSOut
{
    float4 color     : SV_Target;
};
