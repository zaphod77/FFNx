#ifndef __DRIVER_LLGL_H__
#define __DRIVER_LLGL_H__

#define LLGL_ENABLE_UTILITY

#include <LLGL/LLGL.h>
#include <LLGL/Utility.h>
#include <LLGL/Strings.h>
#include <LLGL/Platform/NativeHandle.h>
#include <Gauss/Gauss.h>
#include <Gauss/HLSLTypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "../log.h"

#if defined(__cplusplus)
}
#endif

enum RendererBlendMode {
    BLEND_AVG = 0,
    BLEND_ADD,
    BLEND_SUB,
    BLEND_25P,
    BLEND_NONE
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

enum RendererTextureType
{
    BGRA,
    YUV
};

struct RendererTexture {
    LLGL::Texture* texture = nullptr;
    LLGL::Sampler* sampler = nullptr;
    LLGL::Buffer* data = nullptr;
    uint stride = 0;
};

class Renderer {
private:
    struct Globals {
        float4x4 viewport;
        float4x4 d3dViewport;
        float4x4 d3dProjection;
        float4x4 Flags;
    };

    // Vertex data structure
    struct Vertex
    {
        Gs::Vector4f       position;
        LLGL::ColorRGBAub  color;
        Gs::Vector2f       texcoord0;
    };

    bool compiledSampler;

    Globals globals;
    LLGL::Buffer* constantBuffer = nullptr;

    std::unique_ptr<LLGL::RenderingProfiler> profilerObj_;
    std::unique_ptr<LLGL::RenderingDebugger> debuggerObj_;

    // Default background color
    LLGL::ColorRGBAf backgroundColor = { 0.0f, 0.0f, 0.0f };

    // Render system
    std::unique_ptr<LLGL::RenderSystem> renderer;

    // Main render context
    LLGL::RenderContext* context = nullptr;

    // Main command buffer
    LLGL::CommandBuffer* commands = nullptr;

    // Command queue
    LLGL::CommandQueue* commandQueue = nullptr;

    // Vertex format
    LLGL::VertexFormat vertexInputFormat;

    // Rendering profiler (read only)
    const LLGL::RenderingProfiler& profiler;

    // Graphics pipeline
    LLGL::PipelineState* pipeline = nullptr;
    std::unique_ptr<LLGL::Blob> pipelineCache;
    LLGL::GraphicsPipelineDescriptor pipelineDesc;

    LLGL::PipelineLayout* layout = nullptr;
    LLGL::ResourceHeap* resourceHeap = nullptr;

    uint pipelineTextureCount = 0;
    RendererTexture pipelineTextures[3];

    // Command queue to record and submit command buffers
    LLGL::CommandQueue* queue = nullptr;

    bool IsOpenGL() const;
    bool IsVulkan() const;
    bool IsDirect3D() const;

    float GetAspectRatio() const;

    Gs::Matrix4f PerspectiveProjection(float aspectRatio, float near, float far, float fov);

    void initShaders();

    LLGL::Buffer *vertexBuffer = nullptr;
    uint vertexCount;

    LLGL::Buffer *indexBuffer = nullptr;
    uint indexCount;

    long clearFlags;

public:
    Renderer();

    void init();

    void draw();

    void reset();

    // ---

    LLGL::RenderingCapabilities getCaps();
    uint32_t getSamples();

    void bindVertexBuffer(struct nvertex* inVertex, uint inCount);
    void bindIndexBuffer(word* inIndex, uint inCount);

    void setClearFlags(long flags);
    void setBackgroundColor(float r, float g, float b, float a);

    RendererTexture* createTexture(uint8_t* data, size_t width, size_t height, int stride = 0, RendererTextureType type = RendererTextureType::BGRA);
    void deleteTexture(RendererTexture* rt);
    void useTexture(RendererTexture* rt);

    void isTLVertex(bool flag = false);
    void setBlendMode(RendererBlendMode mode = RendererBlendMode::BLEND_NONE);
    void isFBTexture(bool flag = false);
    void isFullRange(bool flag = false);
    void isTexture(bool flag = false);
    void isYUV(bool flag = false);
    void doInheritTextureAlpha(bool flag = false);

    // Alpha mode emulation
    void setAlphaRef(RendererAlphaFunc func = RendererAlphaFunc::ALWAYS, float ref = 0.0f);
    void doAlphaTest(bool flag = false);

    // Cull face
    void setCullMode(LLGL::CullMode cullMode = LLGL::CullMode::Disabled);

    // Depth test
    void setDepthTest(bool flag = false);
    void setDepthMask(bool flag = false);

    // Wireframe mode
    void setWireframeMode(bool flag = false);

    // Viewport
    void setViweport(const float* matrix);
    void setD3DViweport(const float* matrix);
    void setD3DProjection(const float* matrix);
};

// Log callback
void llgl_log_cb(LLGL::Log::ReportType type, const std::string& message, const std::string& contextInfo, void* userData);

extern Renderer newRenderer;

#endif // !__DRIVER_LLGL_H__
