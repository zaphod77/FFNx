#include "renderer.h"

Renderer newRenderer;

void Renderer_debug_callback(enum DEBUG_MESSAGE_SEVERITY Severity, const Char* Message, const Char* Function, const Char* File, int Line)
{
    switch (Severity) {
    case DEBUG_MESSAGE_SEVERITY::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
    case DEBUG_MESSAGE_SEVERITY::DEBUG_MESSAGE_SEVERITY_ERROR:
        error("%s\n", Message);
        break;
    case DEBUG_MESSAGE_SEVERITY::DEBUG_MESSAGE_SEVERITY_WARNING:
        warning("%s\n", Message);
        break;
    case DEBUG_MESSAGE_SEVERITY::DEBUG_MESSAGE_SEVERITY_INFO:
        info("%s\n", Message);
        break;
    default:
        trace("%s\n", Message);
    }
}

// PRIVATE
void Renderer::setCommonUniforms()
{
    // Map Uniform buffers
    MapHelper<SConstants> SConsts(m_pImmediateContext, m_pShaderConstants, MAP_WRITE, MAP_FLAG_DISCARD);

    SConsts->VSFlags = float4(
        (float)internalState.bIsTLVertex,
        (float)internalState.blendMode,
        (float)internalState.bIsFBTexture,
        (float)internalState.bIsTexture
    );
    if (renderer_debug) trace("%s: VSFlags XYZW(isTLVertex %f, blendMode %f, isFBTexture %f, isTexture %f)\n", __func__, SConsts->VSFlags.x, SConsts->VSFlags.y, SConsts->VSFlags.z, SConsts->VSFlags.w);

    SConsts->FSAlphaFlags = float4(
        (float)internalState.alphaRef,
        (float)internalState.alphaFunc,
        (float)internalState.bDoAlphaTest,
        NULL
    );
    if (renderer_debug) trace("%s: FSAlphaFlags XYZW(inAlphaRef %f, inAlphaFunc %f, bDoAlphaTest %f, NULL)\n", __func__, SConsts->FSAlphaFlags.x, SConsts->FSAlphaFlags.y, SConsts->FSAlphaFlags.z);

    SConsts->FSMiscFlags = float4(
        (float)internalState.bIsMovieFullRange,
        (float)internalState.bIsMovieYUV,
        (float)internalState.bModulateAlpha,
        (float)internalState.bIsMovie
    );
    if (renderer_debug) trace("%s: FSMiscFlags XYZW(isMovieFullRange %f, isMovieYUV %f, modulateAlpha %f, isMovie %f)\n", __func__, SConsts->FSMiscFlags.x, SConsts->FSMiscFlags.y, SConsts->FSMiscFlags.z, SConsts->FSMiscFlags.w);

    SConsts->d3dViewport = internalState.d3dViewMatrix;
    SConsts->d3dProjection = internalState.d3dProjectionMatrix;
    SConsts->worldView = internalState.worldViewMatrix;
    SConsts->orthoProjection = internalState.backendProjMatrix;
}

RENDER_DEVICE_TYPE Renderer::getRendererType()
{
    std::string backend(renderer_backend);
    RENDER_DEVICE_TYPE ret;

    if (backend == "Vulkan") {
        ret = RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_VULKAN;
    }
    else if (backend == "Direct3D12") {
        ret = RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_D3D12;
    }
    else if (backend == "Direct3D11") {
        ret = RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_D3D11;
    }
    else if (backend == "OpenGL") {
        ret = RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_GL;
    }
    else {
        ret = RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_UNDEFINED;
    }

    return ret;
}

void Renderer::destroyAll()
{
    m_pImmediateContext->Flush();

    m_pShaderConstants.Release();

    m_pPSOVertexBuffer.Release();
    m_pPSOIndexBuffer.Release();

    m_pFBSRB.Release();
    m_pFBPSO.Release();

    m_pBBSRB.Release();
    m_pBBPSO.Release();

    m_pFBDepthTexture.Release();
    m_pFBColorTexture.Release();

    m_pPSOPS.Release();
    m_pPSOVS.Release();
    m_pPSOPSPost.Release();
    m_pPSOVSPost.Release();

    m_pImmediateContext.Release();
    m_pSwapChain.Release();
    m_pDevice.Release();
};

void Renderer::reset()
{
    setBackgroundColor();

    setWireframeMode();
    doDepthTest();
    doDepthWrite();
    doScissorTest();
    setCullMode();
    setBlendMode();
    isTLVertex();
    isYUV();
    isFullRange();
    isFBTexture();
    isTexture();
    doModulateAlpha();
    doTextureFiltering();
};

void Renderer::renderFrameBuffer()
{
    /*  y0    y2
     x0 +-----+ x2
        |    /|
        |   / |
        |  /  |
        | /   |
        |/    |
     x1 +-----+ x3
        y1    y3
    */

    // 0
    float x0 = preserve_aspect ? framebufferVertexOffsetX : 0.0f;
    float y0 = 0.0f;
    float u0 = 0.0f;
    float v0 = m_pDevice->GetDeviceCaps().IsGLDevice() ? 1.0f : 0.0f;
    // 1
    float x1 = x0;
    float y1 = game_height;
    float u1 = u0;
    float v1 = m_pDevice->GetDeviceCaps().IsGLDevice() ? 0.0f : 1.0f;
    // 2
    float x2 = x0 + (preserve_aspect ? framebufferVertexWidth : game_width);
    float y2 = y0;
    float u2 = 1.0f;
    float v2 = v0;
    // 3
    float x3 = x2;
    float y3 = y1;
    float u3 = u2;
    float v3 = v1;

    struct nvertex vertices[] = {
        {x0, y0, 1.0f, 1.0f, 0x00000000, 0, u0, v0},
        {x1, y1, 1.0f, 1.0f, 0x00000000, 0, u1, v1},
        {x2, y2, 1.0f, 1.0f, 0x00000000, 0, u2, v2},
        {x3, y3, 1.0f, 1.0f, 0x00000000, 0, u3, v3},
    };
    word indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();

    m_pImmediateContext->SetRenderTargets(1, &pRTV, m_pSwapChain->GetDepthBufferDSV(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state in the immediate context
    m_pImmediateContext->SetPipelineState(m_pBBPSO);

    m_pImmediateContext->ClearRenderTarget(m_pSwapChain->GetCurrentBackBufferRTV(), internalState.clearColorValue.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->ClearDepthStencil(m_pSwapChain->GetDepthBufferDSV(), CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    setCommonUniforms();

    // Create a shader resource binding object and bind all static resources in it
    m_pBBSRB.Release();
    m_pBBPSO->CreateShaderResourceBinding(&m_pBBSRB, true);

    // Set render target color texture SRV in the SRB
    m_pBBSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(pFBColor->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    bindVertexBuffer(vertices, 4);
    bindIndexBuffer(indices, 6);

    // Commit shader resources. This call also sets the shaders in OpenGL backend.
    m_pImmediateContext->CommitShaderResources(m_pBBSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_pImmediateContext->DrawIndexed(PSODrawAttrs);

    m_pImmediateContext->SetRenderTargets(1, &m_pFBColorTexture, m_pFBDepthTexture, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
};

void Renderer::printMatrix(char* name, float4x4 mat)
{
    trace("%s: 0 [%f, %f, %f, %f]\n", name, mat[0], mat[1], mat[2], mat[3]);
    trace("%s: 1 [%f, %f, %f, %f]\n", name, mat[4], mat[5], mat[6], mat[7]);
    trace("%s: 2 [%f, %f, %f, %f]\n", name, mat[8], mat[9], mat[10], mat[11]);
    trace("%s: 3 [%f, %f, %f, %f]\n", name, mat[12], mat[13], mat[14], mat[15]);
};

// PUBLIC

void Renderer::init()
{
    const NativeWindow* NativeWindowHandle = (const NativeWindow*)&hwnd;

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    ShaderCreateInfo ShaderCI;
    SwapChainDesc SCDesc;

    viewWidth = (preserve_aspect ? ((window_size_y * 4) / 3) : window_size_x);
    viewHeight = window_size_y;
    viewOffsetX = (preserve_aspect ? ((window_size_x - viewWidth) / 2) : 0);
    viewOffsetY = 0;

    // In order to prevent weird glitches while rendering we need to use the closest resolution to native's game one
    framebufferWidth = (viewWidth % game_width) ? (viewWidth / game_width + 1) * game_width : viewWidth;
    framebufferHeight = (viewHeight % game_height) ? (viewHeight / game_height + 1) * game_height : viewHeight;

    framebufferVertexWidth = (viewWidth * game_width) / window_size_x;
    framebufferVertexOffsetX = (game_width - framebufferVertexWidth) / 2;

    SCDesc.ColorBufferFormat = TEX_FORMAT_RGBA8_UNORM;
    SCDesc.DepthBufferFormat = TEX_FORMAT_D24_UNORM_S8_UINT;

    // Generic debug callback
    if (renderer_debug) DebugMessageCallback = &Renderer_debug_callback;

    switch (getRendererType())
    {
    case RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_D3D11:
    {
        EngineD3D11CreateInfo EngineCI;
        IEngineFactoryD3D11* pFactoryD3D11 = GetEngineFactoryD3D11();

        if (renderer_debug)
        {
            EngineCI.DebugMessageCallback = &Renderer_debug_callback;
            EngineCI.DebugFlags |=
                D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE |
                D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
        }

        pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
        pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, *NativeWindowHandle, &m_pSwapChain);
        pFactoryD3D11->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    }
    break;

    case RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_D3D12:
    {
        EngineD3D12CreateInfo EngineCI;
        IEngineFactoryD3D12* pFactoryD3D12 = GetEngineFactoryD3D12();

        if (renderer_debug)
        {
            EngineCI.DebugMessageCallback = &Renderer_debug_callback;
            EngineCI.EnableDebugLayer = true;
        }

        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
        pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, *NativeWindowHandle, &m_pSwapChain);
        pFactoryD3D12->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    }
    break;

    case RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_GL:
    {
        EngineGLCreateInfo EngineCI;
        IEngineFactoryOpenGL* pFactoryOpenGL = GetEngineFactoryOpenGL();

        if (renderer_debug) EngineCI.DebugMessageCallback = &Renderer_debug_callback;

        EngineCI.Window = *NativeWindowHandle;
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext, SCDesc, &m_pSwapChain);
        pFactoryOpenGL->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    }
    break;

    case RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_VULKAN:
    {
        EngineVkCreateInfo EngineCI;
        IEngineFactoryVk* pFactoryVk = GetEngineFactoryVk();

        if (renderer_debug)
        {
            EngineCI.DebugMessageCallback = &Renderer_debug_callback;
            EngineCI.EnableValidation = true;
        }

        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
        if (!m_pSwapChain && NativeWindowHandle != nullptr)
            pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, *NativeWindowHandle, &m_pSwapChain);
        pFactoryVk->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    }
    break;

    default:
        error("Unknown device type. Exiting.");
        exit(1);
    }

    // Set common shader properties
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE::SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.UseCombinedTextureSamplers = true;
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    ShaderCI.EntryPoint = "main";

    // VS
    ShaderCI.Desc.Name = "FB VS";
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.FilePath = vertexPath.c_str();
    m_pDevice->CreateShader(ShaderCI, &m_pPSOVS);

    // PS
    ShaderCI.Desc.Name = "FB PS";
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.FilePath = fragmentPath.c_str();
    m_pDevice->CreateShader(ShaderCI, &m_pPSOPS);

    // VS Post
    ShaderCI.Desc.Name = "BB VS";
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.FilePath = vertexPostPath.c_str();
    m_pDevice->CreateShader(ShaderCI, &m_pPSOVSPost);

    // PS Post
    ShaderCI.Desc.Name = "BB PS";
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.FilePath = fragmentPostPath.c_str();
    m_pDevice->CreateShader(ShaderCI, &m_pPSOPSPost);

    // Create shaders uniform buffers
    CreateUniformBuffer(m_pDevice, sizeof(SConstants), "Shader constants", &m_pShaderConstants);

    // Set projection matrix
    internalState.backendProjMatrix = internalState.backendProjMatrix.OrthoOffCenter(0, game_width, game_height, 0, -1.0f, 1.0f, m_pDevice->GetDeviceCaps().IsGLDevice());

    /* === Init FB PSO === */
    FBPSODesc.Name = "FB PSO";
    FBPSODesc.IsComputePipeline = false;
    FBPSODesc.GraphicsPipeline.NumRenderTargets = 1;
    FBPSODesc.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
    FBPSODesc.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;
    FBPSODesc.GraphicsPipeline.pVS = m_pPSOVS;
    FBPSODesc.GraphicsPipeline.pPS = m_pPSOPS;
    FBPSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    FBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    FBPSODesc.GraphicsPipeline.InputLayout.LayoutElements = VSInputLayout;
    FBPSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(VSInputLayout);

    // Define variable type that will be used by default
    FBPSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    // Create FB Color texture
    TextureDesc FBColorDesc;
    FBColorDesc.Type = RESOURCE_DIM_TEX_2D;
    FBColorDesc.Width = framebufferWidth;
    FBColorDesc.Height = framebufferHeight;
    FBColorDesc.MipLevels = 1;
    FBColorDesc.Format = FBPSODesc.GraphicsPipeline.RTVFormats[0];
    FBColorDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
    //FBColorDesc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
    m_pDevice->CreateTexture(FBColorDesc, nullptr, &pFBColor);
    m_pFBColorTexture = pFBColor->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
    
    // Create FB Depth texture
    TextureDesc RTDepthDesc = FBColorDesc;
    RTDepthDesc.Format = FBPSODesc.GraphicsPipeline.DSVFormat;
    RTDepthDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
    m_pDevice->CreateTexture(RTDepthDesc, nullptr, &pFBDepth);
    m_pFBDepthTexture = pFBDepth->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

    // Set Framebuffer texture as renderer targets
    m_pImmediateContext->SetRenderTargets(1, &m_pFBColorTexture, m_pFBDepthTexture, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    /* === Init BB PSO === */

    BBPSODesc.Name = "BB PSO";
    BBPSODesc.IsComputePipeline = false;
    BBPSODesc.GraphicsPipeline.NumRenderTargets = 1;
    BBPSODesc.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
    BBPSODesc.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;
    BBPSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    BBPSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
    BBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    BBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
    BBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNCTION::COMPARISON_FUNC_ALWAYS;
    BBPSODesc.GraphicsPipeline.pVS = m_pPSOVSPost;
    BBPSODesc.GraphicsPipeline.pPS = m_pPSOPSPost;
    BBPSODesc.GraphicsPipeline.InputLayout.LayoutElements = VSInputLayout;
    BBPSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(VSInputLayout);

    // Define variable type that will be used by default
    BBPSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    // Shader variables should typically be mutable, which means they are expected to change on a per-instance basis
    ShaderResourceVariableDesc BBSRVVars[] =
    {
        { SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
    };
    BBPSODesc.ResourceLayout.Variables = BBSRVVars;
    BBPSODesc.ResourceLayout.NumVariables = _countof(BBSRVVars);

    // Define static sampler for g_Texture. Static samplers should be used whenever possible
    StaticSamplerDesc BBSSStaticSamplers[] =
    {
        { SHADER_TYPE_PIXEL, "g_Texture", Sam_LinearClamp }
    };
    BBPSODesc.ResourceLayout.StaticSamplers = BBSSStaticSamplers;
    BBPSODesc.ResourceLayout.NumStaticSamplers = _countof(BBSSStaticSamplers);

    m_pDevice->CreatePipelineState(BBPSODesc, &m_pBBPSO);

    m_pBBPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "SConstants")->Set(m_pShaderConstants);
    m_pBBPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "SConstants")->Set(m_pShaderConstants);

    // Create default empty texture, with id 0
    createTexture(nullptr, 1, 1);
};

void Renderer::shutdown()
{
    destroyAll();

    m_pImmediateContext.Release();
    m_pSwapChain.Release();
    m_pDevice.Release();
}

void Renderer::draw()
{
    if (trace_all) trace("Renderer::%s\n", __func__);

    Viewport viewData;
    Rect scissorData;

    // Prepare texture samplers
    {
        uint idxMax = 3;

        for (uint idx = 0; idx < idxMax; idx++)
        {
            uint rt = internalState.texHandlers[idx];

            if (internalState.textureData[rt].data != nullptr)
            {
                FBShaderSamplers[idx].Desc.MinLOD = -1000;
                FBShaderSamplers[idx].Desc.MaxLOD = 1000;

                if (internalState.bIsMovie)
                {
                    FBShaderSamplers[idx].Desc.AddressU = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_CLAMP;
                    FBShaderSamplers[idx].Desc.AddressV = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_CLAMP;
                    FBShaderSamplers[idx].Desc.AddressW = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_CLAMP;
                }
                else
                {
                    FBShaderSamplers[idx].Desc.AddressU = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_WRAP;
                    FBShaderSamplers[idx].Desc.AddressV = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_WRAP;
                    FBShaderSamplers[idx].Desc.AddressW = TEXTURE_ADDRESS_MODE::TEXTURE_ADDRESS_WRAP;
                }

                if (internalState.bDoTextureFiltering)
                {
                    FBShaderSamplers[idx].Desc.MinFilter = FILTER_TYPE::FILTER_TYPE_LINEAR;
                    FBShaderSamplers[idx].Desc.MagFilter = FILTER_TYPE::FILTER_TYPE_LINEAR;
                }
                else
                {
                    FBShaderSamplers[idx].Desc.MinFilter = FILTER_TYPE::FILTER_TYPE_POINT;
                    FBShaderSamplers[idx].Desc.MagFilter = FILTER_TYPE::FILTER_TYPE_POINT;
                }
            }
        }

        FBPSODesc.ResourceLayout.Variables = FBShaderVars;
        FBPSODesc.ResourceLayout.NumVariables = _countof(FBShaderVars);
        FBPSODesc.ResourceLayout.StaticSamplers = FBShaderSamplers;
        FBPSODesc.ResourceLayout.NumStaticSamplers = _countof(FBShaderSamplers);
    }

    // Set state
    {
        switch (internalState.cullMode)
        {
        case RendererCullMode::FRONT:
            FBPSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE::CULL_MODE_FRONT;
            break;
        case RendererCullMode::BACK: 
            FBPSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE::CULL_MODE_BACK;
            break;
        default:
            FBPSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE::CULL_MODE_NONE;
        }

        switch (internalState.blendMode)
        {
        case RendererBlendMode::BLEND_AVG:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = True;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOp = BLEND_OPERATION::BLEND_OPERATION_ADD;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOpAlpha = BLEND_OPERATION::BLEND_OPERATION_ADD;

            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_INV_SRC_ALPHA;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_INV_SRC_ALPHA;
            break;
        case RendererBlendMode::BLEND_ADD:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = True;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOp = BLEND_OPERATION::BLEND_OPERATION_ADD;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOpAlpha = BLEND_OPERATION::BLEND_OPERATION_ADD;

            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
            break;
        case RendererBlendMode::BLEND_SUB:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = True;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOp = BLEND_OPERATION::BLEND_OPERATION_REV_SUBTRACT;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOpAlpha = BLEND_OPERATION::BLEND_OPERATION_REV_SUBTRACT;

            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
            break;
        case RendererBlendMode::BLEND_25P:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = True;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOp = BLEND_OPERATION::BLEND_OPERATION_ADD;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOpAlpha = BLEND_OPERATION::BLEND_OPERATION_ADD;

            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
            break;
        case RendererBlendMode::BLEND_NONE:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = True;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOp = BLEND_OPERATION::BLEND_OPERATION_ADD;
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendOpAlpha = BLEND_OPERATION::BLEND_OPERATION_ADD;

            if (fancy_transparency)
            {
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_INV_SRC_ALPHA;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_SRC_ALPHA;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_INV_SRC_ALPHA;
            }
            else
            {
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlend = BLEND_FACTOR::BLEND_FACTOR_ONE;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlend = BLEND_FACTOR::BLEND_FACTOR_ZERO;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->SrcBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ONE;
                FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->DestBlendAlpha = BLEND_FACTOR::BLEND_FACTOR_ZERO;
            }
            break;
        default:
            FBPSODesc.GraphicsPipeline.BlendDesc.RenderTargets->BlendEnable = False;
        }

        switch (internalState.primitiveType)
        {
        case RendererPrimitiveType::PT_LINES:
            FBPSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case RendererPrimitiveType::PT_POINTS:
            FBPSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case RendererPrimitiveType::PT_TRIANGLES:
            FBPSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        }

        if (internalState.bDoDepthTest) FBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNCTION::COMPARISON_FUNC_LESS_EQUAL;
        else FBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNCTION::COMPARISON_FUNC_ALWAYS;

        BBPSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = internalState.bDoDepthWrite;
    }

    {
        // Create PSO and bind it
        m_pDevice->CreatePipelineState(FBPSODesc, &m_pFBPSO);

        setCommonUniforms();

        m_pFBPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "SConstants")->Set(m_pShaderConstants);
        m_pFBPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "SConstants")->Set(m_pShaderConstants);

        // Create a shader resource binding object and bind all static resources in it
        m_pFBPSO->CreateShaderResourceBinding(&m_pFBSRB, true);

        viewData.TopLeftX = 0;
        viewData.TopLeftY = 0;
        viewData.Width = framebufferWidth;
        viewData.Height = framebufferHeight;
        viewData.MinDepth = 0;
        viewData.MaxDepth = 1;

        m_pImmediateContext->SetViewports(1, &viewData, viewData.Width, viewData.Height);

        if (internalState.bDoScissorTest)
        {
            scissorData.left = scissorOffsetX;
            scissorData.top = scissorOffsetY;
            scissorData.right = scissorWidth + scissorData.left;
            scissorData.bottom = scissorHeight + scissorData.top;

            FBPSODesc.GraphicsPipeline.RasterizerDesc.ScissorEnable = True;
            m_pImmediateContext->SetScissorRects(1, &scissorData, viewData.Width, viewData.Height);
        }
        else
            FBPSODesc.GraphicsPipeline.RasterizerDesc.ScissorEnable = False;

        // Bind Textures
        {
            uint idxMax = 3;

            for (uint idx = 0; idx < idxMax; idx++)
            {
                uint rt = internalState.texHandlers[idx];

                if (!internalState.bIsMovie && idx > 0) rt = 0;

                m_pFBSRB->GetVariableByName(SHADER_TYPE_PIXEL, FBPSODesc.ResourceLayout.Variables[idx].Name)->Set(
                    internalState.textureData[rt].m_pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE)
                );
            }
        }
    }

    // Set the pipeline state in the immediate context
    m_pImmediateContext->SetPipelineState(m_pFBPSO);

    // Commit shader resources. This call also sets the shaders in OpenGL backend.
    m_pImmediateContext->CommitShaderResources(m_pFBSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_pImmediateContext->DrawIndexed(PSODrawAttrs);

    m_pFBSRB.Release();
    m_pFBPSO.Release();
};

void Renderer::show()
{
    // Reset internal state
    reset();

    renderFrameBuffer();

    m_pSwapChain->Present(enable_vsync);
}

void Renderer::printText(uint16_t x, uint16_t y, uint color, const char* text)
{
    /*
    bgfx::dbgTextPrintf(
        x,
        y,
        color,
        text
    );
    */
}

DeviceCaps Renderer::getCaps()
{
    return m_pDevice->GetDeviceCaps();
}

void Renderer::bindVertexBuffer(struct nvertex* inVertex, uint inCount)
{
    BufferDesc VertBuffDesc;
    BufferData VBData;

    RendererShaderVertex* vertices = new RendererShaderVertex[inCount];

    for (uint idx = 0; idx < inCount; idx++)
    {
        vertices[idx].pos = float4(inVertex[idx]._.x, inVertex[idx]._.y, inVertex[idx]._.z, (std::isinf(inVertex[idx].color.w) ? 1.0f : inVertex[idx].color.w));
        vertices[idx].color = float4(inVertex[idx].color.b / 255.0f, inVertex[idx].color.g / 255.0f, inVertex[idx].color.r / 255.0f, inVertex[idx].color.a / 255.0f);
        vertices[idx].texcoord = float2(inVertex[idx].u, inVertex[idx].v);

        if (vertex_log && idx == 0) trace("%s: %u [XYZW(%f, %f, %f, %f), BGRA(%f, %f, %f, %f), UV(%f, %f)]\n", __func__, idx, vertices[idx].pos.x, vertices[idx].pos.y, vertices[idx].pos.z, vertices[idx].pos.w, vertices[idx].color.b, vertices[idx].color.g, vertices[idx].color.r, vertices[idx].color.a, vertices[idx].texcoord.u, vertices[idx].texcoord.v);
        if (vertex_log && idx == 1) trace("%s: See the rest on RenderDoc.\n", __func__);
    }

    VertBuffDesc.Name = "Vertex Data";
    VertBuffDesc.Usage = USAGE_STATIC;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.uiSizeInBytes = sizeof(RendererShaderVertex) * inCount;

    VBData.pData = vertices;
    VBData.DataSize = VertBuffDesc.uiSizeInBytes;

    m_pPSOVertexBuffer.Release();
    m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_pPSOVertexBuffer);

    Uint32   offset = 0;
    IBuffer* pBuffs[] = { m_pPSOVertexBuffer };
    m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

    delete[] vertices;
};

void Renderer::bindIndexBuffer(word* inIndex, uint inCount)
{
    BufferDesc IndBuffDesc;
    BufferData IBData;

    IndBuffDesc.Name = "Index Data";
    IndBuffDesc.Usage = USAGE_STATIC;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.uiSizeInBytes = sizeof(word) * inCount;

    IBData.pData = inIndex;
    IBData.DataSize = IndBuffDesc.uiSizeInBytes;

    PSODrawAttrs.IndexType = VT_UINT16;
    PSODrawAttrs.NumIndices = inCount;

    m_pPSOIndexBuffer.Release();

    m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_pPSOIndexBuffer);

    m_pImmediateContext->SetIndexBuffer(m_pPSOIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
};

void Renderer::setScissor(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    scissorOffsetX = getInternalCoordX(x);
    scissorOffsetY = getInternalCoordY(y);
    scissorWidth = getInternalCoordX(width);
    scissorHeight = getInternalCoordY(height);
}

void Renderer::setClearFlags(bool doClearColor, bool doClearDepth)
{
    if (doClearColor) m_pImmediateContext->ClearRenderTarget(m_pFBColorTexture, internalState.clearColorValue.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    if (doClearDepth) m_pImmediateContext->ClearDepthStencil(m_pFBDepthTexture, CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Renderer::setBackgroundColor(float r, float g, float b, float a)
{
    internalState.clearColorValue = float4(b, g, r, a);
}

uint Renderer::createTexture(uint8_t* data, size_t width, size_t height, int stride, RendererTextureType type, bool generateMips)
{
    int texId = -1;

    TextureSubResData tSub;
    TextureData tData;
    TextureDesc tDesc;

    for (uint idx = 0; idx < internalState.textureData.size(); idx++)
    {
        if (internalState.textureData[idx].data == nullptr)
        {
            texId = idx;
            break;
        }
    }

    if (texId == -1)
    {
        internalState.textureData.push_back(RendererTexture());
        texId = internalState.textureData.size() - 1;
    }

    size_t size = width * height * (type == RendererTextureType::BGRA ? 4 : 1);
    internalState.textureData[texId].data = new uint8_t[size]{ 0 };

    if (data != nullptr) ::memcpy(internalState.textureData[texId].data, data, size * sizeof(uint8_t));

    tSub.pData = internalState.textureData[texId].data;
    tSub.Stride = stride;
    tSub.DepthStride = 0;

    tData.NumSubresources = 1;
    tData.pSubResources = &tSub;

    //tDesc.MiscFlags = MISC_TEXTURE_FLAG_GENERATE_MIPS;
    tDesc.Type = RESOURCE_DIM_TEX_2D;
    tDesc.Width = width;
    tDesc.Height = height;
    tDesc.Format = (type == RendererTextureType::BGRA ? TEX_FORMAT_RGBA8_UNORM : TEX_FORMAT_R8_UNORM);
    tDesc.Usage = USAGE_STATIC;
    tDesc.BindFlags = BIND_SHADER_RESOURCE;

    m_pDevice->CreateTexture(tDesc, &tData, &internalState.textureData[texId].m_pTexture);

    return texId;
};

void Renderer::deleteTexture(uint texId)
{
    if (texId > 0)
    {
        delete[] internalState.textureData[texId].data;
        internalState.textureData[texId].data = nullptr;

        internalState.textureData[texId].m_pTexture.Release();
    }
};

void Renderer::useTexture(uint texId, uint slot)
{
    if (texId > 0)
    {
        internalState.texHandlers[slot] = texId;
        isTexture(true);
    }
    else
    {
        internalState.texHandlers[slot] = 0;
        isTexture(false);
    }
};

uint Renderer::blitTexture(uint x, uint y, uint width, uint height)
{
    uint dstTex = createTexture(nullptr, framebufferWidth > width ? width : framebufferWidth, framebufferHeight > height ? height : framebufferHeight, 0);
    
    CopyTextureAttribs CopyAttribs(m_pFBColorTexture->GetTexture(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION, internalState.textureData[dstTex].m_pTexture, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Box CopyBox;

    CopyBox.MinX = 0;
    CopyBox.MaxX = x;
    CopyBox.MinY = 0;
    CopyBox.MaxY = y;

    CopyAttribs.pSrcBox = &CopyBox;

    m_pImmediateContext->CopyTexture(CopyAttribs);

    return dstTex;
};

void Renderer::isMovie(bool flag)
{
    internalState.bIsMovie = flag;
};

void Renderer::isTLVertex(bool flag)
{
    internalState.bIsTLVertex = flag;
};

void Renderer::setBlendMode(RendererBlendMode mode)
{
    internalState.blendMode = mode;
};

void Renderer::isTexture(bool flag)
{
    internalState.bIsTexture = flag;
};

void Renderer::isFBTexture(bool flag)
{
    internalState.bIsFBTexture = flag;
};

void Renderer::isFullRange(bool flag)
{
    internalState.bIsMovieFullRange = flag;
};

void Renderer::isYUV(bool flag)
{
    internalState.bIsMovieYUV = flag;
};

void Renderer::doModulateAlpha(bool flag)
{
    internalState.bModulateAlpha = flag;
};

void Renderer::doTextureFiltering(bool flag)
{
    internalState.bDoTextureFiltering = flag;
};

void Renderer::setAlphaRef(RendererAlphaFunc func, float ref)
{
    internalState.alphaFunc = func;
    internalState.alphaRef = ref;
};

void Renderer::doAlphaTest(bool flag)
{
    internalState.bDoAlphaTest = flag;
};

void Renderer::setPrimitiveType(RendererPrimitiveType type)
{
    if (trace_all) trace("%s: %u\n", __func__, type);

    internalState.primitiveType = type;
};

void Renderer::setCullMode(RendererCullMode mode)
{
    internalState.cullMode = mode;
}

void Renderer::doDepthTest(bool flag)
{
    internalState.bDoDepthTest = flag;
}

void Renderer::doDepthWrite(bool flag)
{
    internalState.bDoDepthWrite = flag;
}

void Renderer::doScissorTest(bool flag)
{
    internalState.bDoScissorTest = flag;
}

void Renderer::setWireframeMode(bool flag)
{
    internalState.bUseWireframe = flag;
};

void Renderer::setWorldView(struct matrix *matrix)
{
    ::memcpy(&internalState.worldViewMatrix, &matrix->m[0][0], sizeof(matrix->m));

    if (uniform_log) printMatrix(__func__, internalState.worldViewMatrix);
};

void Renderer::setD3DViweport(struct matrix* matrix)
{
    ::memcpy(&internalState.d3dViewMatrix, &matrix->m[0][0], sizeof(matrix->m));

    if (uniform_log) printMatrix(__func__, internalState.d3dViewMatrix);
};

void Renderer::setD3DProjection(struct matrix* matrix)
{
    ::memcpy(&internalState.d3dProjectionMatrix, &matrix->m[0][0], sizeof(matrix->m));

    if (uniform_log) printMatrix(__func__, internalState.d3dProjectionMatrix);
};

uint16_t Renderer::getInternalCoordX(uint16_t inX)
{
    return (inX * framebufferWidth) / game_width;
}

uint16_t Renderer::getInternalCoordY(uint16_t inY)
{
    return (inY * framebufferHeight) / game_height;
}
