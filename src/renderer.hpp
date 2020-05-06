#ifndef __FFNX_RENDERER_HPP__
#define __FFNX_RENDERER_HPP__

#include "common.h"

#include <bx/math.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <bgfx/platform.h>
#include <bgfx/bgfx.h>

namespace FFNx {
#ifdef DEBUG
    struct RendererCallbacks : public bgfx::CallbackI {
        virtual ~RendererCallbacks()
        {
        }

        virtual void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str) override
        {
            char* error;

            switch (_code) {
            case bgfx::Fatal::Enum::DebugCheck: error = "Debug Check";
            case bgfx::Fatal::Enum::InvalidShader: error = "Invalid Shader";
            case bgfx::Fatal::Enum::UnableToInitialize: error = "Unable To Initialize";
            case bgfx::Fatal::Enum::UnableToCreateTexture: error = "Unable To Create Texture";
            case bgfx::Fatal::Enum::DeviceLost: error = "Device Lost";
            }

            ffDriverLog.write("Backend ERROR: [%s] %s\n", error, _str);
        }

        virtual void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override
        {
            char buffer[16 * 1024];

            va_list argListCopy;
            va_copy(argListCopy, _argList);
            vsnprintf(buffer, sizeof(buffer), _format, argListCopy);
            va_end(argListCopy);

            ffDriverLog.write("Backend TRACE: %s", buffer);
        }

        virtual void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
        {
        }

        virtual void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
        {
        }

        virtual void profilerEnd() override
        {
        }

        virtual uint32_t cacheReadSize(uint64_t _id) override
        {
            // Shader not found
            return 0;
        }

        virtual bool cacheRead(uint64_t _id, void* _data, uint32_t _size) override
        {
            // Rebuild Shader
            return false;
        }

        virtual void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) override
        {
        }

        virtual void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t /*_size*/, bool _yflip) override
        {
        }

        virtual void captureBegin(uint32_t _width, uint32_t _height, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool _yflip) override
        {
        }

        virtual void captureEnd() override
        {
        }

        virtual void captureFrame(const void* _data, uint32_t /*_size*/) override
        {
        }

    };
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

    enum RendererCommittedTextureType
    {
        Y,
        U,
        V,
        BGRA
    };

    typedef struct
    {
        float x;
        float y;
        float z;
        float w;
        uint32_t bgra;
        float u;
        float v;
    } RendererShaderVertex;

    class Renderer {
    private:
        template<typename T>
        uint32_t vectorSizeOf(const typename std::vector<T>& vec)
        {
            return sizeof(T) * vec.size();
        }

        struct RendererState
        {
            uint16_t texCount = 0;

            float alphaRef = 0.0f;
            RendererAlphaFunc alphaFunc;

            bool bIsTLVertex = false;
            bool bIsFBTexture = false;
            bool bDoTextureFiltering = false;
            bool bDoAlphaTest = false;
            bool bInheritTextureAlpha = false;

            float d3dViewMatrix[4][4];
            float d3dProjectionMatrix[4][4];
            float worldViewMatrix[4][4];

            RendererBlendMode blendMode = RendererBlendMode::BLEND_NONE;

            uint64_t state = BGFX_STATE_MSAA;

            bgfx::FrameBufferHandle framebufferHandle = BGFX_INVALID_HANDLE;
        };

        char shaderTextureBindings[3][6] = {
            "tex", // BGRA or Y share the same binding
            "tex_u",
            "tex_v",
        };

        std::string vertexPath = ffUserConfig.getShaderVertexPath();
        std::string fragmentPath = ffUserConfig.getShaderFragmentPath();

        bgfx::ShaderHandle vertexShader = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle fragmentShader = BGFX_INVALID_HANDLE;

        bgfx::ViewId backendViewId = 0;
        bgfx::ProgramHandle backendProgramHandle = BGFX_INVALID_HANDLE;
        uint32_t clearColorValue;

        bgfx::VertexBufferHandle vertexBufferHandle = BGFX_INVALID_HANDLE;
        bgfx::IndexBufferHandle indexBufferHandle = BGFX_INVALID_HANDLE;
        bgfx::VertexLayout vertexLayout;

        std::vector<bgfx::UniformHandle> bgfxUniformHandles;

        uint32_t internalStateIdx = 0;
        std::vector<RendererState> internalStates;

#ifdef DEBUG
        RendererCallbacks bgfxCallbacks;
#endif

        bool bIsMovie = false;
        bool bIsMovieFullRange = false;
        bool bIsReadyToRender = false;

        float backendProjMatrix[16];

        float viewOffsetX = 0.0f;
        float viewOffsetY = 0.0f;
        float viewWidth = 0.0f;
        float viewHeight = 0.0f;

        void setMovieQuadVertex()
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
            float x0 = 0.0f;
            float y0 = 0.0f;
            float u0 = 0.0f;
            float v0 = 0.0f;
            // 1
            float x1 = x0;
            float y1 = viewHeight;
            float u1 = 0.0f;
            float v1 = 1.0f;
            // 2
            float x2 = viewWidth;
            float y2 = y0;
            float u2 = 1.0f;
            float v2 = 0.0f;
            // 3
            float x3 = x2;
            float y3 = y1;
            float u3 = 1.0f;
            float v3 = 1.0f;

            std::vector<RendererShaderVertex> vertices = {
                {x0, y0, 0.0, 1.0f, 0x00000000, u0, v0},
                {x1, y1, 0.0, 1.0f, 0x00000000, u1, v1},
                {x2, y2, 0.0, 1.0f, 0x00000000, u2, v2},
                {x3, y3, 0.0, 1.0f, 0x00000000, u3, v3},
            };

            std::vector<uint16_t> indices =
            {
                0, 1, 2,
                1, 3, 2
            };

            setVertexData(vertices);
            setIndexData(indices);
        }

        void setCommonUniforms(bool inIsTexture = false, bool inIsTLVertex = false, bool inUseYUV = false)
        {
            const float VSFlags[4] = {
                inIsTLVertex, // isTLVertex
                internalStates[internalStateIdx].blendMode, // blendMode
                internalStates[internalStateIdx].bIsFBTexture, // isFBTexture
                NULL // NOT USED
            };
            //ffDriverLog.write("%s: VSFlags XYZW(isTLVertex %f, blendMode %f, isFBTexture %f, NULL)", __func__, VSFlags[0], VSFlags[1], VSFlags[2]);

            const float FSFlags[][4] = {
                bIsMovieFullRange, inIsTexture, inUseYUV, internalStates[internalStateIdx].bIsFBTexture, // isFullRange, isTexture, isYUV, isFBTexture
                internalStates[internalStateIdx].alphaRef, internalStates[internalStateIdx].alphaFunc, internalStates[internalStateIdx].bDoAlphaTest, internalStates[internalStateIdx].bInheritTextureAlpha, // inAlphaRef, inAlphaFunc, doAlphaTest, inheritTextureAlpha
                NULL, NULL, NULL, NULL, // NOT USED, NOT USED, NOT USED, NOT USED
                NULL, NULL, NULL, NULL  // NOT USED, NOT USED, NOT USED, NOT USED
                
            };
            //ffDriverLog.write("%s: FSFlags [0] XYZW(isFullRange %f, isTexture %f, isYUV %f, isFBTexture %f)", __func__, FSFlags[0][0], FSFlags[0][1], FSFlags[0][2], FSFlags[0][3]);
            //glLoadMatrixffDriverLog.write("%s: FSFlags [1] XYZW(inAlphaRef %f, inAlphaFunc %f, bDoAlphaTest %f, inheritTextureAlpha %f)", __func__, FSFlags[1][0], FSFlags[1][1], FSFlags[1][2], FSFlags[1][3]);

            setUniform("VSFlags", bgfx::UniformType::Vec4, VSFlags);
            setUniform("FSFlags", bgfx::UniformType::Mat4, FSFlags);

            setUniform("d3dViewport", bgfx::UniformType::Mat4, internalStates[internalStateIdx].d3dViewMatrix);
            setUniform("d3dProjection", bgfx::UniformType::Mat4, internalStates[internalStateIdx].d3dProjectionMatrix);
        }

        bgfx::RendererType::Enum getRendererType() {
            std::string backend = ffUserConfig.getBackend();
            bgfx::RendererType::Enum ret;

            if (backend == "Vulkan") {
                ret = bgfx::RendererType::Vulkan;
            }
            else if (backend == "DirectX 12") {
                ret = bgfx::RendererType::Direct3D12;
            }
            else if (backend == "DirectX 11") {
                ret = bgfx::RendererType::Direct3D11;
            }
            else if (backend == "DirectX 9") {
                ret = bgfx::RendererType::Direct3D9;
            }
            else if (backend == "OpenGL") {
                ret = bgfx::RendererType::OpenGL;
            }
            else {
                ret = bgfx::RendererType::Noop;
            }

            return ret;
        }

        // Via https://dev.to/pperon/hello-bgfx-4dka
        bgfx::ShaderHandle getShader(const char* filePath)
        {
            bgfx::ShaderHandle handle = BGFX_INVALID_HANDLE;

            FILE* file = fopen(filePath, "rb");

            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            const bgfx::Memory* mem = bgfx::alloc(fileSize);
            fread(mem->data, 1, fileSize, file);
            fclose(file);

            handle = bgfx::createShader(mem);

            if (bgfx::isValid(handle))
            {
                bgfx::setName(handle, filePath);
            }

            return handle;
        }

        bgfx::UniformHandle getUniform(std::string uniformName, bgfx::UniformType::Enum uniformType)
        {
            bgfx::UniformHandle handle = bgfx::createUniform(uniformName.c_str(), uniformType);

            bgfxUniformHandles.push_back(
                handle
            );

            return handle;
        }

        bgfx::UniformHandle setUniform(const char* uniformName, bgfx::UniformType::Enum uniformType, const void* uniformValue)
        {
            bgfx::UniformHandle handle = getUniform(uniformName, uniformType);

            if (bgfx::isValid(handle))
            {
                bgfx::setUniform(handle, uniformValue);
            }

            return handle;
        }

        void destroyUniforms() {
            for (uint32_t idx = 0; idx < bgfxUniformHandles.size(); idx++)
            {
                if (bgfx::isValid(bgfxUniformHandles[idx]))
                    bgfx::destroy(bgfxUniformHandles[idx]);
            }

            bgfxUniformHandles.clear();
            bgfxUniformHandles.shrink_to_fit();
        }

        void destroyAll() {
            destroyUniforms();

            bgfx::destroy(vertexBufferHandle);

            bgfx::destroy(indexBufferHandle);

            bgfx::destroy(backendProgramHandle);
        }

        void doRender(uint16_t inIdx) {
            bgfx::setViewRect(backendViewId, viewOffsetX, viewOffsetY, viewWidth, viewHeight);
            bgfx::setViewTransform(backendViewId, internalStates[internalStateIdx].worldViewMatrix, backendProjMatrix);

            setCommonUniforms(
                (internalStates[internalStateIdx].texCount >= 0),
                bIsMovie || internalStates[internalStateIdx].bIsTLVertex,
                bIsMovie && internalStates[internalStateIdx].texCount == 3
            );

            if (!bIsMovie)
            {
                if (inIdx > 0)
                {
                    bgfx::TextureHandle handle = { inIdx };

                    uint32_t flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

                    if (internalStates[internalStateIdx].bDoTextureFiltering)
                        flags |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;
                    else
                        flags |= BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;

                    bgfx::setTexture(0, getUniform(shaderTextureBindings[0], bgfx::UniformType::Sampler), handle, flags);
                }
            } else
                setMovieQuadVertex();

            internalStates[internalStateIdx].state |= BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;

            bgfx::setState(internalStates[internalStateIdx].state);

            bgfx::submit(backendViewId, backendProgramHandle);

            destroyUniforms();
        }

    public:
        Renderer() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        void init() {
            viewWidth = (ffUserConfig.keepAspectRatio() ? ((ffWindow.getHeight() / 3.0f) * 4.0f) : ffWindow.getWidth());
            viewHeight = ffWindow.getHeight();
            viewOffsetX = (ffUserConfig.keepAspectRatio() ? ((ffWindow.getWidth() - viewWidth) / 2.0f) : 0.0f);
            viewOffsetY = 0.0f;

            bgfx::Init bgfxInit;
            bgfxInit.platformData.nwh = ffWindow.getHWnd();
            bgfxInit.type = getRendererType();
            bgfxInit.resolution.width = viewWidth;
            bgfxInit.resolution.height = viewHeight;

            if (ffUserConfig.wantsVSync())
                bgfxInit.resolution.reset = BGFX_RESET_VSYNC;

#ifdef DEBUG
            bgfxInit.debug = true;
            bgfxInit.callback = &bgfxCallbacks;
#endif

            if (!bgfx::init(bgfxInit)) exit(1);

            ffWindow.resetWindowTitle();

            bx::mtxOrtho(backendProjMatrix, 0.0f, viewWidth, viewHeight, 0.0f, 1.0f, -1.0f, 0.0, bgfx::getCaps()->homogeneousDepth);

            // Create Program
            vertexShader = getShader(vertexPath.c_str());
            fragmentShader = getShader(fragmentPath.c_str());

            backendProgramHandle = bgfx::createProgram(
                vertexShader,
                fragmentShader,
                true
            );

            vertexLayout
                .begin()
                .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();

            start();

            bgfx::frame();
        }

        void shutdown() {
            destroyAll();

            bgfx::shutdown();
        }

        void start() {
            internalStates.push_back(
                RendererState()
            );

            internalStateIdx = internalStates.size() - 1;

            ffDriverLog.write("%s: current state index %lu", __func__, internalStateIdx);
        }

        void end() {
            bgfx::frame();

            if (bgfx::isValid(internalStates[internalStateIdx].framebufferHandle))
                bgfx::destroy(internalStates[internalStateIdx].framebufferHandle);

            internalStates.pop_back();

            internalStateIdx = internalStates.size() - 1;

            ffDriverLog.write("%s: current state index %lu", __func__, internalStateIdx);
        }

        // ---

        float getInternalCoordX(float inX)
        {
            return (inX * viewWidth) / 640.0f;
        }

        float getInternalCoordY(float inY)
        {
            return (inY * viewHeight) / 480.0f;
        }

        uint32_t getMaxTextureSize() {
            return bgfx::getCaps()->limits.maxTextureSize;
        }

        uint32_t getMaxTextureSamplers() {
            return bgfx::getCaps()->limits.maxTextureSamplers;
        }

        void enableWireframe() {
            bgfx::setDebug(BGFX_DEBUG_WIREFRAME);
        }

        void enableTextureFiltering(bool flag = false) {
            internalStates[internalStateIdx].bDoTextureFiltering = flag;
        }

        void enableAlphaTest(bool flag = false) {
            internalStates[internalStateIdx].bDoAlphaTest = flag;
        }

        void isMovie(bool flag = false) {
            bIsMovie = flag;
        }

        void isMovieFullRange(bool flag = false) {
            bIsMovieFullRange = flag;
        }

        void setBlendMode(RendererBlendMode inBlendMode) {
            internalStates[internalStateIdx].blendMode = inBlendMode;
        }

        void setAlphaFunc(RendererAlphaFunc inAlphaFunc = RendererAlphaFunc::ALWAYS, float inAlphaRef = 0.0f) {
            internalStates[internalStateIdx].alphaFunc = inAlphaFunc;
            internalStates[internalStateIdx].alphaRef = inAlphaRef;
        }

        void setState(uint64_t inState = BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_GREATER) {
            internalStates[internalStateIdx].state |= inState;
        }

        void unsetState(uint64_t inState) {
            internalStates[internalStateIdx].state &= ~inState;
        }

        void setRendererViewport(uint16_t inViewportX, uint16_t inViewportY, uint16_t inViewportWidth, uint16_t inViewportHeight) {
            //ffDriverLog.write("%s: XYWH(%u, %u, %u, %u)", __func__, inViewportX, inViewportY, inViewportWidth, inViewportHeight);

            bgfx::setViewScissor(backendViewId, inViewportX + viewOffsetX, inViewportY + viewOffsetY, inViewportWidth + viewOffsetX, inViewportHeight + viewOffsetY);
        }

        void setRendererFramebuffer(uint32_t inWidth, uint32_t inHeight, FFNx::RendererCommittedTextureType inType = FFNx::RendererCommittedTextureType::BGRA) {
            ffDriverLog.write("Calling function %s", __func__);

            bgfx::TextureFormat::Enum texType = inType == FFNx::RendererCommittedTextureType::BGRA ? bgfx::TextureFormat::BGRA8 : bgfx::TextureFormat::R8;

            internalStates[internalStateIdx].framebufferHandle = bgfx::createFrameBuffer(
                inWidth,
                inHeight,
                texType
            );

            bgfx::setViewFrameBuffer(
                backendViewId,
                internalStates[internalStateIdx].framebufferHandle
            );

            internalStates[internalStateIdx].bIsFBTexture = true;
        }

        void setWorldViewMatrix(float inMatrix[4][4]) {
            for (int row = 0; row < 4; row++)
            {
                for (int col = 0; col < 4; col++)
                {
                    internalStates[internalStateIdx].worldViewMatrix[row][col] = inMatrix[row][col];
                }
            }
        }

        void setD3DViewMatrix(float inMatrix[4][4]) {
            for (int row = 0; row < 4; row++)
            {
                for (int col = 0; col < 4; col++)
                {
                    internalStates[internalStateIdx].d3dViewMatrix[row][col] = inMatrix[row][col];
                }
            }
        }

        void setD3DProjectionMatrix(float inMatrix[4][4]) {
            for (int row = 0; row < 4; row++)
            {
                for (int col = 0; col < 4; col++)
                {
                    internalStates[internalStateIdx].d3dProjectionMatrix[row][col] = inMatrix[row][col];
                }
            }
        }

        void setVertexData(std::vector<RendererShaderVertex> inData) {
            if (bgfx::isValid(vertexBufferHandle)) bgfx::destroy(vertexBufferHandle);

            vertexBufferHandle = bgfx::createVertexBuffer(
                bgfx::copy(
                    inData.data(),
                    vectorSizeOf(inData)
                ),
                vertexLayout
            );

            bgfx::setVertexBuffer(0, vertexBufferHandle);
        }

        void setIndexData(std::vector<uint16_t> inData) {
            if (bgfx::isValid(indexBufferHandle)) bgfx::destroy(indexBufferHandle);

            indexBufferHandle = bgfx::createIndexBuffer(
                bgfx::copy(
                    inData.data(),
                    vectorSizeOf(inData)
                )
            );

            bgfx::setIndexBuffer(indexBufferHandle);
        }

        void isTLVertex(bool flag = false) {
            internalStates[internalStateIdx].bIsTLVertex = flag;
        }

        // ---

        void setClearColor(uint8_t inR = 0, uint8_t inG = 0, uint8_t inB = 0, uint8_t inA = 0) {
            clearColorValue = FFNx::Utils::createBGRA(inR, inG, inB, inA);

            ffDriverLog.write("%s: 0x%08x", __func__, clearColorValue);
        }

        uint16_t commitTexture(std::string inName, uint32_t inWidth, uint32_t inHeight, uint32_t inX = 0, uint32_t inY = 0, void* inData = NULL, uint32_t inDataSize = 0, uint32_t inStride = 0, bool inHasMips = false, RendererCommittedTextureType inTexType = RendererCommittedTextureType::BGRA, uint64_t inFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE) {
            ffDriverLog.write("%s: Ready to commit %s", __func__, inName.c_str());

            const bgfx::Memory* mem = bgfx::copy(inData, inDataSize);
            bgfx::TextureFormat::Enum textureFormat = bgfx::TextureFormat::R8;
            short texBindingIdx = 0;

            if (inTexType == RendererCommittedTextureType::BGRA)
                textureFormat = bgfx::TextureFormat::BGRA8;
            else
                texBindingIdx = inTexType; // Index inherited from the texture type

            bgfx::TextureHandle handle = bgfx::createTexture2D(
                inWidth,
                inHeight,
                inHasMips,
                1,
                textureFormat,
                inFlags,
                inStride > 0 ? NULL : mem
            );

            if (bgfx::isValid(handle)) {
                bgfx::setName(handle, (inName + std::to_string(handle.idx)).c_str());

                if (inStride > 0)
                    bgfx::updateTexture2D(
                        handle,
                        1,
                        0,
                        inX,
                        inY,
                        inWidth,
                        inHeight,
                        mem,
                        inStride
                    );
            }

            if (bIsMovie)
                bgfx::setTexture(texBindingIdx, getUniform(shaderTextureBindings[texBindingIdx], bgfx::UniformType::Sampler), handle, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

            internalStates[internalStateIdx].texCount++;

            ffDriverLog.write("%s: Texture %s%u committed.", __func__, inName.c_str(), handle.idx);

            return handle.idx;
        }

        void destroyTexture(uint16_t inIdx) {
            if (inIdx > 0)
            {
                bgfx::TextureHandle handle = { inIdx };

                if (bgfx::isValid(handle)) {
                    bgfx::destroy(handle);

                    ffDriverLog.write("%s: destroyed.", __func__, inIdx);
                }
            }
        }

        void draw(uint16_t inIdx = 0) {
            if (!inIdx)
                ffDriverLog.write("%s: empty screen", __func__);

            doRender(inIdx);
        }

        void clear(bool doClearColor = false, bool doClearDepth = false) {
            uint16_t clearFlags = BGFX_CLEAR_NONE;

            if (doClearColor)
                clearFlags |= BGFX_CLEAR_COLOR;

            if (doClearDepth)
                clearFlags |= BGFX_CLEAR_DEPTH;
            
            bgfx::setViewClear(backendViewId, clearFlags, clearColorValue, 0.0f);

            // Create an empty frame only when movies finish to play
            if (bIsMovie) bgfx::touch(backendViewId);
        }

    };
};

#endif
