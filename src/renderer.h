#pragma once

#include <vector>
#include <map>

#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Primitives/interface/DebugOutput.h>

#include <DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#include <DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>

#include <DiligentCore/Graphics/GraphicsTools/interface/CommonlyUsedStates.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>

#include <DiligentCore/Graphics/GraphicsTools/interface/GraphicsUtilities.h>

#include "log.h"

using namespace Diligent;

enum RendererBlendMode {
    BLEND_AVG = 0,
    BLEND_ADD,
    BLEND_SUB,
    BLEND_25P,
    BLEND_NONE
};

enum RendererCullMode {
    DISABLED = 0,
    FRONT,
    BACK
};

enum RendererAlphaFunc {
    NEVER = 0,
    LESS,
    EQUAL,
    LEQUAL,
    GREATER,
    NOTEQUAL,
    GEQUAL,
    ALWAYS
};

enum RendererPrimitiveType
{
    PT_POINTS = 0,
    PT_LINES,
    PT_LINE_LOOP,
    PT_LINE_STRIP,
    PT_TRIANGLES,
    PT_TRIANGLE_STRIP,
    PT_TRIANGLE_FAN,
    PT_QUADS,
    PT_QUAD_STRIP
};

enum RendererTextureType
{
    BGRA = 0,
    YUV,
};

enum RendererInternalType
{
    RGBA8 = 0,
    COMPRESSED_RGBA
};

void Renderer_debug_callback(enum DEBUG_MESSAGE_SEVERITY Severity, const Char* Message, const Char* Function, const Char* File, int Line);

class Renderer {
private:
    struct RendererTexture
    {
        uint8_t* data = nullptr;
        RefCntAutoPtr<ITexture> m_pTexture;
    };

    struct RendererShaderVertex
    {
        float4 pos;
        float4 color;
        float2 texcoord;
    };

    struct SConstants
    {
        float4 VSFlags;
        float4 FSAlphaFlags;
        float4 FSMiscFlags;
        float4x4 orthoProjection;
        float4x4 d3dViewport;
        float4x4 d3dProjection;
        float4x4 worldView;
    };

    struct RendererState
    {
        uint texHandlers[3];

        std::vector<RendererTexture> textureData;

        bool bUseWireframe = false;

        bool bDoAlphaTest = false;
        float alphaRef = 0.0f;
        RendererAlphaFunc alphaFunc;

        bool bDoDepthTest = false;
        bool bDoDepthWrite = false;
        bool bDoScissorTest = false;

        bool bIsTLVertex = false;
        bool bIsFBTexture = false;
        bool bIsTexture = false;
        bool bDoTextureFiltering = false;
        bool bModulateAlpha = false;
        bool bIsMovie = false;
        bool bIsMovieFullRange = false;
        bool bIsMovieYUV = false;

        float4x4 backendProjMatrix;

        float4x4 d3dViewMatrix;
        float4x4 d3dProjectionMatrix;
        float4x4 worldViewMatrix;

        float4 clearColorValue;

        RendererCullMode cullMode = RendererCullMode::DISABLED;
        RendererBlendMode blendMode = RendererBlendMode::BLEND_NONE;
        RendererPrimitiveType primitiveType = RendererPrimitiveType::PT_TRIANGLES;
    };

    std::string vertexPath = "shaders/FFNx.vsh";
    std::string fragmentPath = "shaders/FFNx.psh";
    std::string vertexPostPath = "shaders/FFNx.post.vsh";
    std::string fragmentPostPath = "shaders/FFNx.post.psh";

    RefCntAutoPtr<IRenderDevice>           m_pDevice;
    RefCntAutoPtr<IDeviceContext>          m_pImmediateContext;
    RefCntAutoPtr<ISwapChain>              m_pSwapChain;

    RefCntAutoPtr<IPipelineState>          m_pFBPSO;
    RefCntAutoPtr<IShaderResourceBinding>  m_pFBSRB;

    RefCntAutoPtr<IPipelineState>          m_pBBPSO;
    RefCntAutoPtr<IShaderResourceBinding>  m_pBBSRB;

    RefCntAutoPtr<IShader> m_pPSOVS;
    RefCntAutoPtr<IShader> m_pPSOPS;
    RefCntAutoPtr<IShader> m_pPSOVSPost;
    RefCntAutoPtr<IShader> m_pPSOPSPost;

    RefCntAutoPtr<IBuffer> m_pShaderConstants;

    RefCntAutoPtr<IBuffer> m_pPSOVertexBuffer;
    RefCntAutoPtr<IBuffer> m_pPSOIndexBuffer;

    // Framebuffer render target and depth-stencil
    RefCntAutoPtr<ITexture> pFBColor;
    RefCntAutoPtr<ITexture> pFBDepth;
    RefCntAutoPtr<ITextureView> m_pFBColorTexture;
    RefCntAutoPtr<ITextureView> m_pFBDepthTexture;

    PipelineStateDesc FBPSODesc;
    PipelineStateDesc BBPSODesc;
    
    DrawIndexedAttribs PSODrawAttrs;

    // Define vertex shader input layout
    LayoutElement VSInputLayout[3]
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 4, VT_FLOAT32, False},
        // Attribute 1 - texture color
        LayoutElement{1, 0, 4, VT_FLOAT32, False},
        // Attribute 2 - texture coordinates
        LayoutElement{2, 0, 2, VT_FLOAT32, False}
    };

    ShaderResourceVariableDesc FBShaderVars[3] =
    {
        { SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
        { SHADER_TYPE_PIXEL, "g_Texture_u", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
        { SHADER_TYPE_PIXEL, "g_Texture_v", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
    };

    // Define static sampler for g_Texture. Static samplers should be used whenever possible
    StaticSamplerDesc FBShaderSamplers[3] =
    {
        { SHADER_TYPE_PIXEL, "g_Texture", Sam_LinearWrap },
        { SHADER_TYPE_PIXEL, "g_Texture_u", Sam_LinearWrap },
        { SHADER_TYPE_PIXEL, "g_Texture_v", Sam_LinearWrap }
    };

    RendererState internalState;

    Uint32 viewOffsetX = 0;
    Uint32 viewOffsetY = 0;
    Uint32 viewWidth = 0;
    Uint32 viewHeight = 0;

    Uint32 framebufferWidth = 0;
    Uint32 framebufferHeight = 0;

    Uint32 scissorOffsetX = 0;
    Uint32 scissorOffsetY = 0;
    Uint32 scissorWidth = 0;
    Uint32 scissorHeight = 0;

    Uint32 framebufferVertexOffsetX = 0;
    Uint32 framebufferVertexWidth = 0;

    void setCommonUniforms();
    RENDER_DEVICE_TYPE getRendererType();
    void destroyAll();

    void reset();

    void renderFrameBuffer();

    void printMatrix(char* name, float4x4 mat);

public:
    void init();
    void shutdown();

    void draw();
    void show();

    void printText(uint16_t x, uint16_t y, uint attr, const char* text);

    // ---

    DeviceCaps getCaps();

    void bindVertexBuffer(struct nvertex* inVertex, uint inCount);
    void bindIndexBuffer(word* inIndex, uint inCount);

    void setScissor(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void setClearFlags(bool doClearColor = false, bool doClearDepth = false);
    void setBackgroundColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);

    uint createTexture(uint8_t* data, size_t width, size_t height, int stride = 0, RendererTextureType type = RendererTextureType::BGRA, bool generateMips = false);
    void deleteTexture(uint texId);
    void useTexture(uint texId, uint slot = 0);
    uint blitTexture(uint x, uint y, uint width, uint height);

    void isMovie(bool flag = false);
    void isTLVertex(bool flag = false);
    void setBlendMode(RendererBlendMode mode = RendererBlendMode::BLEND_NONE);
    void isTexture(bool flag = false);
    void isFBTexture(bool flag = false);
    void isFullRange(bool flag = false);
    void isYUV(bool flag = false);
    void doModulateAlpha(bool flag = false);
    void doTextureFiltering(bool flag = false);

    // Alpha mode emulation
    void setAlphaRef(RendererAlphaFunc func = RendererAlphaFunc::ALWAYS, float ref = 0.0f);
    void doAlphaTest(bool flag = false);

    // Internal states
    void setPrimitiveType(RendererPrimitiveType type = RendererPrimitiveType::PT_TRIANGLES);
    void setCullMode(RendererCullMode mode = RendererCullMode::DISABLED);
    void doDepthTest(bool flag = false);
    void doDepthWrite(bool flag = false);

    // Scissor test
    void doScissorTest(bool flag = false);

    // Wireframe mode
    void setWireframeMode(bool flag = false);

    // Viewport
    void setWorldView(struct matrix* matrix);
    void setD3DViweport(struct matrix* matrix);
    void setD3DProjection(struct matrix* matrix);

    // Internal coord calculation
    uint16_t getInternalCoordX(uint16_t inX);
    uint16_t getInternalCoordY(uint16_t inY);
};

extern Renderer newRenderer;
