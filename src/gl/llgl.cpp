#include "llgl.h"

Renderer newRenderer;

// GLOBAL
void llgl_log_cb(LLGL::Log::ReportType type, const std::string& message, const std::string& contextInfo, void* userData)
{
    switch (type)
    {
    case LLGL::Log::ReportType::Error:
        debug_printf(nullptr, true, text_colors[TEXTCOLOR_RED], "%s (%s)\n", message.c_str(), contextInfo.c_str());
        break;
    case LLGL::Log::ReportType::Warning:
        debug_printf(nullptr, true, text_colors[TEXTCOLOR_YELLOW], "%s (%s)\n", message.c_str(), contextInfo.c_str());
        break;
    case LLGL::Log::ReportType::Information:
        debug_printf(nullptr, true, text_colors[TEXTCOLOR_WHITE], "%s (%s)\n", message.c_str(), contextInfo.c_str());
        break;
    case LLGL::Log::ReportType::Performance:
        debug_printf(nullptr, true, text_colors[TEXTCOLOR_GREEN], "%s (%s)\n", message.c_str(), contextInfo.c_str());
        break;
    }
};

// PRIVATE

bool Renderer::IsOpenGL() const
{
    return (renderer->GetRendererID() == LLGL::RendererID::OpenGL);
}

bool Renderer::IsVulkan() const
{
    return (renderer->GetRendererID() == LLGL::RendererID::Vulkan);
}

bool Renderer::IsDirect3D() const
{
    return
        (
            renderer->GetRendererID() == LLGL::RendererID::Direct3D9 ||
            renderer->GetRendererID() == LLGL::RendererID::Direct3D10 ||
            renderer->GetRendererID() == LLGL::RendererID::Direct3D11 ||
            renderer->GetRendererID() == LLGL::RendererID::Direct3D12
            );
}

float Renderer::GetAspectRatio() const
{
    auto resolution = context->GetVideoMode().resolution;
    return (static_cast<float>(resolution.width) / static_cast<float>(resolution.height));
}

Gs::Matrix4f Renderer::PerspectiveProjection(float inAspectRatio, float inNear, float inFar, float inFov)
{
    int flags = (IsOpenGL() || IsVulkan() ? Gs::ProjectionFlags::UnitCube : 0);
    return Gs::ProjectionMatrix4f::Perspective(inAspectRatio, inNear, inFar, inFov, flags).ToMatrix4();
}

void Renderer::initShaders()
{
    // Define vertex input format
    vertexInputFormat.AppendAttribute({ "a_position", LLGL::Format::RGBA32Float, 0 });
    vertexInputFormat.AppendAttribute({ "a_color", LLGL::Format::RGBA8UNorm, 1 });
    vertexInputFormat.AppendAttribute({ "a_texcoord0", LLGL::Format::RG32Float, 2 });

    // Create shaders
    LLGL::Shader* vertShader = nullptr;
    LLGL::Shader* fragShader = nullptr;

    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

    LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

    if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "shaders/FFNx.vert" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "shaders/FFNx.frag" };
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
    {
        vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, "shaders/FFNx.vert.spv");
        fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "shaders/FFNx.frag.spv");
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "shaders/FFNx.hlsl", "VSMain", "vs_4_0" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "shaders/FFNx.hlsl", "PSMain", "ps_4_0" };
    }

    // Specify vertex attributes for vertex shader
    vertShaderDesc.vertex.inputAttribs = vertexInputFormat.attributes;

    vertShader = renderer->CreateShader(vertShaderDesc);
    fragShader = renderer->CreateShader(fragShaderDesc);

    for (auto shader : { vertShader, fragShader })
    {
        if (shader != nullptr)
        {
            std::string log = shader->GetReport();
            if (!log.empty())
                error("%s\n", log.c_str());
        }
    }

    // Create shader program which is used as composite
    LLGL::ShaderProgramDescriptor shaderProgramDesc;
    {
        shaderProgramDesc.vertexShader = vertShader;
        shaderProgramDesc.fragmentShader = fragShader;
    }
    LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

    // Link shader program and check for errors
    if (shaderProgram->HasErrors())
        error("%s\n", shaderProgram->GetReport().c_str());

    pipelineDesc.shaderProgram = shaderProgram;
}

// PUBLIC

Renderer::Renderer()
:
    profilerObj_ { new LLGL::RenderingProfiler() },
    debuggerObj_ { new LLGL::RenderingDebugger() },
    profiler { *profilerObj_ }
{};

void Renderer::init()
{
    // Set report callback to standard output
    LLGL::Log::SetReportCallback(&llgl_log_cb);

    // Set up renderer descriptor
    LLGL::RenderSystemDescriptor rendererDesc = renderer_backend;

    // Set debug callback
    rendererDesc.debugCallback = [](const std::string& type, const std::string& message)
    {
        trace("[%s] %s\n", type.c_str(), message.c_str());
    };

    // Create render system
    renderer = LLGL::RenderSystem::Load(
        rendererDesc,
        profilerObj_.get(),
        debuggerObj_.get()
    );

    // Create render context
    LLGL::RenderContextDescriptor contextDesc;
    {
        contextDesc.videoMode.resolution = { (uint32_t)window_size_x, (uint32_t)window_size_y };
        contextDesc.vsync.enabled = enable_vsync;
        contextDesc.samples = 8;
    }
    context = renderer->CreateRenderContext(contextDesc);

    // Create command buffer
    commands = renderer->CreateCommandBuffer();

    // Get command queue
    commandQueue = renderer->GetCommandQueue();

    // Print renderer information
    const auto& info = renderer->GetRendererInfo();
    const auto contextRes = context->GetResolution();

    trace("render system:\n");
    trace("  renderer: %s\n", info.rendererName.c_str());
    trace("  device: %s\n", info.deviceName.c_str());
    trace("  vendor: %s\n", info.vendorName.c_str());
    trace("  shading language: %s\n", info.shadingLanguageName.c_str());
    trace("render context:\n");
    trace("  resolution: %dx%d\n", contextRes.width, contextRes.height);
    trace("  samples: %d\n", context->GetSamples());
    trace("  colorFormat: %s\n", LLGL::ToString(context->GetColorFormat()));
    trace("  depthStencilFormat: %s\n", LLGL::ToString(context->GetDepthStencilFormat()));

    if (!info.extensionNames.empty())
    {
        trace("extensions:\n");
        for (const auto& name : info.extensionNames) trace("  %s\n", name.c_str());
    }

    // Set window title and show window
    auto& window = LLGL::CastTo<LLGL::Window>(context->GetSurface());
    auto wndDesc = window.GetDesc();
    LLGL::NativeContextHandle nctxHandle;

    nctxHandle.parentWindow = hwnd;
    wndDesc.windowContext = (const void*)(&nctxHandle);
    wndDesc.resizable = false;

    window.SetDesc(wndDesc);

    // Initialize default projection matrix
    globals.viewport = PerspectiveProjection(GetAspectRatio(), 1.0f, -1.0f, 0.0f);

    // Initialize shaders
    initShaders();
    
    // Initialize pipeline layout
    compiledSampler = (renderer->GetRendererID() == LLGL::RendererID::OpenGL);
    if (compiledSampler)
        layout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc(
                "cbuffer(globals@0):frag:vert, texture(tex@1):frag, sampler(tex@1):frag, texture(tex_u@2):frag, sampler(tex_u@2):frag, texture(tex_v@3):frag, sampler(tex_v@3):frag"
            )
        );
    else
        layout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc(
                "cbuffer(0):frag:vert, texture(1):frag, sampler(2):frag, texture(3):frag, sampler(4):frag, texture(5):frag, sampler(6):frag"
            )
        );

    // Initialize pipeline descriptor
    pipelineDesc.renderPass = context->GetRenderPass();
    pipelineDesc.rasterizer.multiSampleEnabled = (contextDesc.samples > 1);
    pipelineDesc.pipelineLayout = layout;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;

    // Create globals constant buffer
    constantBuffer = renderer->CreateBuffer(
        LLGL::ConstantBufferDesc(sizeof(globals)),
        &globals
    );

    // Set defaults
    reset();
};

void Renderer::draw()
{
    // Create graphics PSO
    pipeline = renderer->CreatePipelineState(pipelineDesc);

    // Create resource heap
    LLGL::ResourceHeapDescriptor resHeapDesc;
    resHeapDesc.pipelineLayout = layout;
    resHeapDesc.resourceViews = {
        constantBuffer,
        (pipelineTextureCount > 0) ? pipelineTextures[0].texture : nullptr,
        (pipelineTextureCount > 0) ? pipelineTextures[0].sampler : nullptr,
        (pipelineTextureCount > 1) ? pipelineTextures[1].texture : nullptr,
        (pipelineTextureCount > 1) ? pipelineTextures[1].sampler : nullptr,
        (pipelineTextureCount > 2) ? pipelineTextures[2].texture : nullptr,
        (pipelineTextureCount > 2) ? pipelineTextures[2].sampler : nullptr
    };

    resourceHeap = renderer->CreateResourceHeap(resHeapDesc);

    // Get command queue to record and submit command buffers
    if (queue == nullptr) queue = renderer->GetCommandQueue();

    // Create command buffer to submit subsequent graphics commands to the GPU
    if (commands == nullptr) commands = renderer->CreateCommandBuffer();

    // Begin recording commands
    commands->Begin();
    {
        // Expand textures if required
        for (uint idx = 0; idx < pipelineTextureCount; idx++)
        {
            if (pipelineTextures[idx].stride > 0)
            {
                LLGL::TextureRegion texRegion;
                texRegion.extent.width = pipelineTextures[idx].texture->GetDesc().extent.width;
                texRegion.extent.height = pipelineTextures[idx].texture->GetDesc().extent.height;
                texRegion.extent.depth = 1;

                commands->CopyTextureFromBuffer(
                    *pipelineTextures[idx].texture,
                    texRegion,
                    *pipelineTextures[idx].data,
                    0,
                    pipelineTextures[idx].stride
                );
            }
        }

        commands->UpdateBuffer(*constantBuffer, 0, &globals, sizeof(globals));

        // Set vertex buffer
        commands->SetVertexBuffer(*vertexBuffer);

        // Set index buffer
        commands->SetIndexBuffer(*indexBuffer);

        // Set the render context as the initial render target
        commands->BeginRenderPass(*context);
        {
            // Set background color
            commands->SetClearColor(backgroundColor);

            // Clear color buffer
            commands->Clear(clearFlags);

            // Set viewport and scissor rectangle
            commands->SetViewport(context->GetVideoMode().resolution);

            // Set graphics pipeline
            commands->SetPipelineState(*pipeline);

            // Set resource heap
            commands->SetResourceHeap(*resourceHeap);

            // Draw triangle with 3 vertices
            commands->DrawIndexed(indexCount, 0);
        }
        commands->EndRenderPass();
    }
    commands->End();
    queue->Submit(*commands);

    // Present the result on the screen
    context->Present();

    // Free memory
    renderer->Release(*pipeline);
    renderer->Release(*resourceHeap);

    pipelineTextureCount = 0;
};

void Renderer::reset()
{
    clearFlags = 0;
    backgroundColor = { 0.0f, 0.0f, 0.0f };

    isTLVertex();
    setBlendMode();
    isFBTexture();
    isFullRange();
    isTexture();
    isYUV();
    doInheritTextureAlpha();
    setAlphaRef();
    doAlphaTest();
};

LLGL::RenderingCapabilities Renderer::getCaps()
{
    return renderer->GetRenderingCaps();
};

uint32_t Renderer::getSamples()
{
    return context->GetSamples();
};

void Renderer::bindVertexBuffer(struct nvertex* inVertex, uint inCount)
{
    vertexCount = inCount;
    Vertex *vertices = new Vertex[vertexCount];

    // Copy vertex data
    for (uint idx = 0; idx < vertexCount; idx++)
    {
        vertices[idx].position = { inVertex[idx]._.x, inVertex[idx]._.y, inVertex[idx]._.z, inVertex[idx].color.w };
        vertices[idx].color = { inVertex[idx].color.r, inVertex[idx].color.g, inVertex[idx].color.b, inVertex[idx].color.a };
        vertices[idx].texcoord0 = { inVertex[idx].u, inVertex[idx].v };
    }

    // Update stride in case out vertex structure is not 4-byte aligned
    vertexInputFormat.SetStride(sizeof(Vertex));

    // Create vertex buffer
    LLGL::BufferDescriptor vertexBufferDesc;
    {
        vertexBufferDesc.size = sizeof(Vertex) * vertexCount;
        vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
        vertexBufferDesc.vertexAttribs = vertexInputFormat.attributes;
    }
    vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

    ::free(vertices);
};

void Renderer::bindIndexBuffer(word* inIndex, uint inCount)
{
    indexCount = inCount;

    // Create index buffer
    LLGL::BufferDescriptor indexBufferDesc;
    {
        indexBufferDesc.size = sizeof(word) * indexCount;
        indexBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
        indexBufferDesc.format = LLGL::Format::R16UInt;
    }
    indexBuffer = renderer->CreateBuffer(indexBufferDesc, inIndex);
};

void Renderer::setClearFlags(long flags)
{
    clearFlags |= flags;
}

void Renderer::setBackgroundColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f)
{
    backgroundColor.r = r;
    backgroundColor.g = g;
    backgroundColor.b = b;
    backgroundColor.a = a;
}

RendererTexture* Renderer::createTexture(uint8_t* data, size_t width, size_t height, int stride, RendererTextureType type)
{
    RendererTexture *ret = new RendererTexture();

    LLGL::TextureDescriptor     texDesc;
    LLGL::SamplerDescriptor     samplerDesc;

    texDesc.miscFlags = LLGL::MiscFlags::NoInitialData;
    texDesc.extent.width = width;
    texDesc.extent.height = height;
    texDesc.extent.depth = 1;
    
    if (type == RendererTextureType::BGRA)
        texDesc.format = LLGL::Format::BGRA8UInt;
    else
        texDesc.format = LLGL::Format::R8UInt;
    
    uint32_t dataSize = LLGL::GetMemoryFootprint(texDesc.format, texDesc.extent.width * texDesc.extent.height * texDesc.extent.depth);

    if (stride > 0)
    {        
        LLGL::BufferDescriptor texBufDesc;
        texBufDesc.size = dataSize;
        texBufDesc.bindFlags = LLGL::BindFlags::CopySrc;
        
        ret->stride = stride;
        ret->data = renderer->CreateBuffer(texBufDesc, data);

        texDesc.bindFlags = LLGL::BindFlags::CopyDst | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::Sampled;
    }

    ret->texture = renderer->CreateTexture(texDesc);
    ret->sampler = renderer->CreateSampler(samplerDesc);

    return ret;
};

void Renderer::deleteTexture(RendererTexture* rt)
{
    if (rt)
    {
        if (rt->texture) renderer->Release(*rt->texture);
        if (rt->sampler) renderer->Release(*rt->sampler);
        if (rt->data) renderer->Release(*rt->data);

        delete rt;
    }
};

void Renderer::useTexture(RendererTexture* rt)
{
    pipelineTextures[pipelineTextureCount] = *rt;

    pipelineTextureCount++;
};

void Renderer::isTLVertex(bool flag)
{
    globals.Flags.At(0, 0) = flag;
};

void Renderer::setBlendMode(RendererBlendMode mode)
{
    globals.Flags.At(0, 1) = mode;
};

void Renderer::isFBTexture(bool flag)
{
    globals.Flags.At(0, 2) = flag;
};

void Renderer::isFullRange(bool flag)
{
    globals.Flags.At(1, 0) = flag;
};

void Renderer::isTexture(bool flag)
{
    globals.Flags.At(1, 1) = flag;
};

void Renderer::isYUV(bool flag)
{
    globals.Flags.At(1, 2) = flag;
};

void Renderer::doInheritTextureAlpha(bool flag)
{
    globals.Flags.At(1, 3) = flag;
};

void Renderer::setAlphaRef(RendererAlphaFunc func, float ref)
{
    globals.Flags.At(2, 0) = ref;
    globals.Flags.At(2, 1) = func;
};

void Renderer::doAlphaTest(bool flag)
{
    globals.Flags.At(2, 2) = flag;
};

void Renderer::setCullMode(LLGL::CullMode cullMode)
{
    pipelineDesc.rasterizer.cullMode = cullMode;
};

void Renderer::setDepthTest(bool flag)
{
    pipelineDesc.depth.testEnabled = flag;
};

void Renderer::setDepthMask(bool flag)
{
    pipelineDesc.depth.writeEnabled = flag;
};

void Renderer::setWireframeMode(bool flag)
{
    pipelineDesc.rasterizer.polygonMode = flag ? LLGL::PolygonMode::Wireframe : LLGL::PolygonMode::Fill;
};

void Renderer::setViweport(const float* matrix)
{
    ::memcpy(globals.viewport.Ptr(), matrix, sizeof(globals.viewport));
};

void Renderer::setD3DViweport(const float* matrix)
{
    ::memcpy(globals.d3dViewport.Ptr(), matrix, sizeof(globals.d3dViewport));
};

void Renderer::setD3DProjection(const float* matrix)
{
    ::memcpy(globals.d3dProjection.Ptr(), matrix, sizeof(globals.d3dProjection));
};
