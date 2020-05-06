#ifndef __FF7DRIVER_ENGINE_HPP__
#define __FF7DRIVER_ENGINE_HPP__

#include "main.hpp"

namespace FF7 {

    /*
        * Render states supported by the graphics engine
        *
        * Not all of these were implemented in the original games, even fewer are
        * actually required. Named after corresponding Direct3D states.
        */
    enum effects
    {
        V_WIREFRAME,			// 0x1
        V_TEXTURE,				// 0x2
        V_LINEARFILTER,			// 0x4
        V_PERSPECTIVE,			// 0x8
        V_TMAPBLEND,			// 0x10
        V_WRAP_U,				// 0x20
        V_WRAP_V,				// 0x40
        V_UNKNOWN80,			// 0x80
        V_COLORKEY,				// 0x100
        V_DITHER,				// 0x200
        V_ALPHABLEND,			// 0x400
        V_ALPHATEST,			// 0x800
        V_ANTIALIAS,			// 0x1000
        V_CULLFACE,				// 0x2000
        V_NOCULL,				// 0x4000
        V_DEPTHTEST,			// 0x8000
        V_DEPTHMASK,			// 0x10000
        V_SHADEMODE,			// 0x20000
        V_SPECULAR,				// 0x40000
        V_LIGHTSTATE,			// 0x80000
        V_FOG,					// 0x100000
        V_TEXADDR,				// 0x200000
        V_UNKNOWN400000,        // 0x400000
        V_UNKNOWN800000,        // 0x800000
        V_ALPHAFUNC,            // 0x1000000
        V_ALPHAREF,             // 0x2000000
    };

    class Engine {
    private:
        TextureSet* (*createTextureSet)();
        Palette* (*createPaletteForTex)(u32, TextureHeader*, TextureSet*);
        void* (*sub_665D9A)(Matrix*, nVertex*, IndexedPrimitive*, AuxillaryGFX*, DrawableState*, GameContext*);
        void* (*sub_671742)(u32, AuxillaryGFX*, DrawableState*);
        void* (*sub_6B27A9)(Matrix*, IndexedPrimitive*, PolygonSet*, AuxillaryGFX*, PGroup*, void*, GameContext*);
        void* (*sub_68D2B8)(u32, PolygonSet*, void*);
        void* (*sub_665793)(Matrix*, u32, IndexedPrimitive*, PolygonSet*, AuxillaryGFX*, PGroup*, GameContext*);
        void* (*matrix3x4)(Matrix*);
        void* (*sub_6A2865)(void*);

        BlendMode blendModes[5] = {      // PSX blend mode:
            {1, 1, 0x80, 5, 0x10, 6, 0x20, 0, 0}, // average
            {1, 0, 0xFF, 2, 2,    2, 2,    0, 0}, // additive blending
            {1, 0, 0xFF, 4, 8,    2, 2,    0, 0}, // subtractive blending
            {1, 0, 0x40, 5, 0x10, 2, 2,    0, 0}, // 25%? incoming color
            {1, 0, 0xFF, 2, 2,    1, 1,    0, 0}, // 
        };

        std::string currentTexName;
        uint32_t currentTexIndex = 0;
        uint64_t currentTextureFilterFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;
        uint32_t currentAlphaRef = 0;
        bool cullModeCW = false;

        // helper function used to draw a set of triangles without palette data
        void genericDraw(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext, u32 vertexType)
        {
            internalRenderVertex(
                NULL,
                indexVert->vertices,
                indexVert->vertexCount,
                indexVert->indices,
                indexVert->indexCount,
                vertexType == FF7_VERTEXTYPE_TLVERTEX,
                polygonSet->specialDamageFlags
            );
        }

        // helper function used to draw a set of triangles with palette data
        void genericDrawPaletted(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext, u32 vertexType)
        {
            uint32_t count = indexVert->count;
            uint8_t* palettes = indexVert->palettes;
            AuxillaryGFX* hundred_data = polygonSet->auxillaries;
            nVertex* vertices;
            nVertex* _vertices = indexVert->vertices;
            u16* indices = indexVert->indices;

            if (!polygonSet->drainedHP) return;

            while (count > 0)
            {
                uint32_t palette_index = *palettes++;
                uint32_t var30 = 1;
                uint32_t vertexcount = indexVert->graphicsObject->verticesPerShape;
                uint32_t indexcount = indexVert->graphicsObject->indicesPerShape;

                vertices = _vertices;

                hundred_data->textureSet->paletteIndex = palette_index;

                paletteChanged(0, 0, 0, hundred_data->textureSet->palette, hundred_data->textureSet);

                while (var30 < count)
                {
                    if (*palettes != palette_index) break;

                    palettes++;

                    vertexcount += indexVert->graphicsObject->verticesPerShape;
                    indexcount += indexVert->graphicsObject->indicesPerShape;

                    var30++;
                }

                _vertices = &_vertices[indexVert->graphicsObject->verticesPerShape * var30];

                count -= var30;

                internalRenderVertex(
                    NULL,
                    indexVert->vertices,
                    indexVert->vertexCount,
                    indexVert->indices,
                    indexVert->indexCount,
                    vertexType == FF7_VERTEXTYPE_TLVERTEX,
                    polygonSet->specialDamageFlags
                );
            }
        }

        void multiplyMatrix(Matrix* a, Matrix* b, Matrix* dest)
        {

#define MMUL(I, J, N) a->elements[I - 1][N - 1] * b->elements[N - 1][J - 1]
#define MMUL1(I, J) dest->elements[I - 1][J - 1] = MMUL(I, J, 1) + MMUL(I, J, 2) + MMUL(I, J, 3) + MMUL(I, J, 4)
#define MMULROW(I) MMUL1(I, 1); MMUL1(I, 2); MMUL1(I, 3); MMUL1(I, 4)

            MMULROW(1);
            MMULROW(2);
            MMULROW(3);
            MMULROW(4);
        }

        // helper function to set simple render states (single parameter)
        void internalSetRenderState(uint32_t state, uint32_t option, GameContext* gameContext)
        {
            switch (state)
            {
                // wireframe rendering, not used?
            case V_WIREFRAME:
                ffDriverLog.write("%s: V_WIREFRAME -> option: %u", __func__, option);
                if (option)
                    ffRenderer.enableWireframe();
                break;

                // texture filtering, can be disabled globally via config file
            case V_LINEARFILTER:
                ffDriverLog.write("%s: V_LINEARFILTER -> option: %u", __func__, option);
                if (option && !(gameContext->field_988))
                    ffRenderer.enableTextureFiltering(true);
                else
                    ffRenderer.enableTextureFiltering(false);
                break;

                // perspective correction should never be turned off
            case V_PERSPECTIVE:
                ffDriverLog.write("%s: V_PERSPECTIVE -> option: %u", __func__, option);
                // noop
                break;

                // color keying is done when textures are converted, not when rendering
            case V_COLORKEY:
                ffDriverLog.write("%s: V_COLORKEY -> option: %u", __func__, option);
                // noop
                break;

                // no dithering necessary in 32-bit color mode
            case V_DITHER:
                ffDriverLog.write("%s: V_DITHER -> option: %u", __func__, option);
                // noop
                break;

                // alpha test is used in many places in FF8 instead of color keying
            case V_ALPHATEST:
                ffDriverLog.write("%s: V_ALPHATEST -> option: %u", __func__, option);
                if (option)
                    ffRenderer.enableAlphaTest(true);
                else
                    ffRenderer.enableAlphaTest(false);
                break;

                // cull face, does this ever change?
            case V_CULLFACE:
                ffDriverLog.write("%s: V_CULLFACE -> option: %u", __func__, option);
                if (option)
                {
                    ffRenderer.setState(BGFX_STATE_CULL_CW);
                    cullModeCW = true;
                }
                else
                {
                    ffRenderer.setState(BGFX_STATE_CULL_CCW);
                    cullModeCW = false;
                }
                break;

                // turn off culling completely, once again unsure if its ever used
            case V_NOCULL:
                ffDriverLog.write("%s: V_NOCULL -> option: %u", __func__, option);
                if (option)
                    ffRenderer.unsetState(
                        cullModeCW ? BGFX_STATE_CULL_CW : BGFX_STATE_CULL_CCW
                    );
                else
                    ffRenderer.setState(BGFX_STATE_CULL_CCW);
                break;

                // turn depth testing on/off
            case V_DEPTHTEST:
                ffDriverLog.write("%s: V_DEPTHTEST -> option: %u", __func__, option);
                if (option)
                    ffRenderer.setState(BGFX_STATE_DEPTH_TEST_GEQUAL);
                else
                    ffRenderer.unsetState(BGFX_STATE_DEPTH_TEST_GEQUAL);
                break;

                // depth mask, enable/disable writing to the Z-buffer
            case V_DEPTHMASK:
                ffDriverLog.write("%s: V_DEPTHMASK -> option: %u", __func__, option);
                if (option)
                    ffRenderer.setState(BGFX_STATE_WRITE_Z);
                else
                    ffRenderer.unsetState(BGFX_STATE_WRITE_Z);
                break;

                // no idea what this is supposed to do
            case V_TEXADDR:
                ffDriverLog.write("%s: V_TEXADDR -> option: %u", __func__, option);
                // noop
                break;

                // function and reference values for alpha test
            case V_ALPHAFUNC:
                ffDriverLog.write("%s: V_ALPHAFUNC -> option: %u", __func__, option);
                switch (option)
                {
                    case 0:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::NEVER, currentAlphaRef / 255.0f);
                        break;
                    case 1:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::ALWAYS, currentAlphaRef / 255.0f);
                        break;
                    case 2:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::LESS, currentAlphaRef / 255.0f);
                        break;
                    case 3:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::LEQUAL, currentAlphaRef / 255.0f);
                        break;
                    case 4:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::EQUAL, currentAlphaRef / 255.0f);
                        break;
                    case 5:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::GEQUAL, currentAlphaRef / 255.0f);
                        break;
                    case 6:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::GREATER, currentAlphaRef / 255.0f);
                        break;
                    case 7:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::NOTEQUAL, currentAlphaRef / 255.0f);
                        break;
                    default:
                        ffRenderer.setAlphaFunc(FFNx::RendererAlphaFunc::LEQUAL, currentAlphaRef / 255.0f);
                        break;
                }
            case V_ALPHAREF:
                ffDriverLog.write("%s: V_ALPHAREF -> option: %u", __func__, option);
                currentAlphaRef = option;
                break;
            default:
                break;
            }
        }

        void internalSetBlendMode(FFNx::RendererBlendMode blendMode) {
            ffRenderer.setBlendMode(blendMode);

            ffRenderer.setState(BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD));

            switch (blendMode)
            {
            case FFNx::RendererBlendMode::BLEND_AVG:
                ffDriverLog.write("%s: BLEND_AVG", __func__);
                ffRenderer.setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
                break;
            case FFNx::RendererBlendMode::BLEND_ADD:
                ffDriverLog.write("%s: BLEND_ADD", __func__);
                ffRenderer.setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE));
                break;
            case FFNx::RendererBlendMode::BLEND_SUB:
                ffDriverLog.write("%s: BLEND_SUB", __func__);
                ffRenderer.setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE));
                ffRenderer.setState(BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB));
                break;
            case FFNx::RendererBlendMode::BLEND_25P:
                ffDriverLog.write("%s: BLEND_25P", __func__);
                ffRenderer.setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE));
                break;
            case FFNx::RendererBlendMode::BLEND_NONE:
                ffDriverLog.write("%s: BLEND_NONE", __func__);
                ffRenderer.setState(BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO));
                break;
            default:
                ffDriverLog.write("%s: Unknown blend mode %u", __func__, blendMode);
            }
        }

        void internalPassVertices(nVertex* vertices, uint32_t count) {
            std::vector<FFNx::RendererShaderVertex> vVertices(count);

            for (uint32_t idx = 0; idx < count; idx++)
            {
                vVertices[idx].x = ffRenderer.getInternalCoordX(vertices[idx]._.x);
                vVertices[idx].y = ffRenderer.getInternalCoordY(vertices[idx]._.y);
                vVertices[idx].z = vertices[idx]._.z;
                vVertices[idx].w = vertices[idx].color.w;
                vVertices[idx].bgra = vertices[idx].color.color;
                vVertices[idx].u = vertices[idx].u;
                vVertices[idx].v = vertices[idx].v;

                //ffDriverLog.write("%s: %u { XYZW(%f, %f, %f, %f), BGRA(0x%08lx), UV(%f, %f) }", __func__, idx, vVertices[idx].x, vVertices[idx].y, vVertices[idx].z, vVertices[idx].w, vVertices[idx].bgra, vVertices[idx].u, vVertices[idx].v);
            }

            ffRenderer.setVertexData(vVertices);
        }

        void internalPassIndices(uint16_t* indices, uint32_t count) {
            std::vector<uint16_t> vIndices(indices, indices + count);

            std::string outStr;
            for (uint32_t idx = 0; idx < vIndices.size(); idx++)
            {
                outStr.append( std::to_string(vIndices[idx]) );

                if ( idx != vIndices.size()-1)
                    outStr.append(", ");

            }
            //ffDriverLog.write("%s: [ %s ]", __func__, outStr.c_str());

            ffRenderer.setIndexData(vIndices);
        }

        // convert a single 8-bit paletted pixel to 32-bit BGRA format
        uint32_t internalPal2Bgra(uint32_t pixel, uint32_t* palette, uint32_t paletteOffset, uint32_t colorKey, uint32_t referenceAlpha)
        {
            if (colorKey && pixel == 0)
                return 0;
            else
            {
                uint32_t color = palette[paletteOffset + pixel];
                // FF7 uses a form of alpha keying to emulate PSX blending
                if (FF7DRIVER_BGRA_A(color) == 0xFE) color = (color & 0xFFFFFF) | referenceAlpha;
                return color;
            }
        }

        // convert an entire image from its native format to 32-bit BGRA
        uint32_t* internalConvertImageData(uint8_t* imageData, uint32_t w, uint32_t h, TextureFormat* textureFormat, bool invertAlpha, bool colorKey, uint32_t paletteOffset, uint32_t referenceAlpha)
        {
            uint32_t i, j, o = 0, c = 0;
            uint32_t* convertedImageData = new uint32_t[w * h * 4];

            // paletted source data (4-bit palettes are expanded to 8-bit by the game)
            if (textureFormat->bytesperpixel == 1)
            {
                ffDriverLog.write("%s: paletted source data, %u bytes per pixel", __func__, textureFormat->bytesperpixel);

                if (!textureFormat->usePalette)
                {
                    ffDriverLog.write("%s: unsupported texture format", __func__);
                    return NULL;
                }

                for (i = 0; i < w; i++)
                {
                    for (j = 0; j < h; j++)
                    {
                        if (imageData[o] > textureFormat->paletteSize)
                        {
                            ffDriverLog.write("%s: texture conversion error", __func__);
                            return NULL;
                        }

                        convertedImageData[c++] = internalPal2Bgra(imageData[o++], textureFormat->paletteData, paletteOffset, colorKey, referenceAlpha);
                    }
                }
            }
            // RGB(A) source data
            else
            {
                if (textureFormat->usePalette)
                {
                    ffDriverLog.write("%s: unsupported texture format", __func__);
                    return NULL;
                }

                ffDriverLog.write("%s: RGBA source data, %u bytes per pixel", __func__, textureFormat->bytesperpixel);

                for (i = 0; i < w; i++)
                {
                    for (j = 0; j < h; j++)
                    {
                        uint32_t pixel = 0;
                        uint32_t color = 0;

                        switch (textureFormat->bytesperpixel)
                        {
                            // 16-bit RGB(A)
                        case 2:
                            pixel = *((WORD*)(&imageData[o]));
                            break;
                            // 24-bit RGB
                        case 3:
                            pixel = imageData[o] | imageData[o + 1] << 8 | imageData[o + 2] << 16;
                            break;
                            // 32-bit RGBA or RGBX
                        case 4:
                            pixel = *((uint32_t*)(&imageData[o]));
                            break;

                        default:
                            ffDriverLog.write("%s: unsupported texture format", __func__);
                            return NULL;
                        }

                        o += textureFormat->bytesperpixel;

                        // PSX style mask bit
                        if (colorKey && (pixel & ~textureFormat->alphaMask) == 0)
                        {
                            convertedImageData[c++] = 0;
                            continue;
                        }

                        // convert source data to 8 bits per channel
                        color = textureFormat->blueMax > 0 ? ((((pixel & textureFormat->blueMask) >> textureFormat->blueShift) * 255) / textureFormat->blueMax) : 0;
                        color |= (textureFormat->greenMax > 0 ? ((((pixel & textureFormat->greenMask) >> textureFormat->greenShift) * 255) / textureFormat->greenMax) : 0) << 8;
                        color |= (textureFormat->redMax > 0 ? ((((pixel & textureFormat->redMask) >> textureFormat->redShift) * 255) / textureFormat->redMax) : 0) << 16;

                        // special case to deal with poorly converted PSX images in FF7
                        if (invertAlpha && pixel != 0x8000) color |= (textureFormat->alphaMax > 0 ? (255 - ((((pixel & textureFormat->alphaMask) >> textureFormat->alphaShift) * 255) / textureFormat->alphaMax)) : 255) << 24;
                        else color |= (textureFormat->alphaMax > 0 ? ((((pixel & textureFormat->alphaMask) >> textureFormat->alphaShift) * 255) / textureFormat->alphaMax) : 255) << 24;

                        convertedImageData[c++] = color;
                    }
                }
            }

            return convertedImageData;
        }

        void internalRenderVertex(uint64_t inPrimitiveTopology, nVertex *vertices, uint32_t vertexCount, uint16_t *indices, uint32_t indexCount, bool isTLVertex, bool enableScissor) {
            if (inPrimitiveTopology != NULL) ffRenderer.setState(inPrimitiveTopology);
            ffRenderer.isTLVertex(isTLVertex);
            internalPassVertices(vertices, vertexCount);
            internalPassIndices(indices, indexCount);
            // TODO: WANTS MIPMAP

            ffDriverLog.write("%s: draw %s%u", __func__, currentTexName.c_str(), currentTexIndex);
            ffRenderer.draw(currentTexIndex);
        }

    public:
        Engine() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        u32 createDriver(GameContext* gameContext) {
            ffRenderer.init();

            TextureFormat* texture_format = gameContext->externals->createTextureFormat();
            gameContext->externals->makePixelFormat(32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000, texture_format);
            gameContext->externals->listAddTextureFormat(texture_format, gameContext);

            // Store internal functions
            createTextureSet = gameContext->externals->createTextureSet;
            createPaletteForTex = gameContext->externals->createTexturePalette;
            sub_6A2865 = (void* (*)(void*))gameContext->externals->sub_6A2865;
            sub_665D9A = (void* (*)(Matrix*, nVertex*, IndexedPrimitive*, AuxillaryGFX*, DrawableState*, GameContext*))FF7_FN_SUB_665D9A;
            sub_671742 = (void* (*)(u32, AuxillaryGFX*, DrawableState*))FF7_FN_SUB_671742;
            sub_6B27A9 = (void* (*)(Matrix*, IndexedPrimitive*, PolygonSet*, AuxillaryGFX*, PGroup*, void*, GameContext*))FF7_FN_SUB_6B27A9;
            sub_68D2B8 = (void* (*)(u32, PolygonSet*, void*))FF7_FN_SUB_68D2B8;
            sub_665793 = (void* (*)(Matrix*, u32, IndexedPrimitive*, PolygonSet*, AuxillaryGFX*, PGroup*, GameContext*))FF7_FN_SUB_665793;
            matrix3x4 = (void* (*)(Matrix*))FF7_FN_MATRIX3X4;

            return true;
        }

        void destroyDriver(GameContext* gameContext) {
            ffPatch.unreplaceFunctions();
            
            ffRenderer.shutdown();
            
            ffUserConfig.flush();
        }

        u32 lock(u32 surface) {
            return true;
        }

        u32 unlock(u32 surface) {
            return true;
        }

        void flip(GameContext* gameContext) {
            if (!ffUserConfig.wantsFullscreen()) ShowCursor(true);
        }

        u32 beginScene(u32 unk, GameContext* gameContext) {
            gameContext->in_scene += 1;

            field_84(unk, gameContext);

            ffRenderer.start();

            return true;
        }

        void endScene(GameContext* gameContext) {
            gameContext->in_scene -= 1;

            ffRenderer.end();
        }

        void clear(u32 clearColor, u32 clearDepth, u32 unk, GameContext* gameContext) {
            GameMode* currentMode = ff7DetectGameMode.getMode();

            ffRenderer.clear(
                clearColor || currentMode->mode == FF7::GameModes::MODE_MENU,
                clearDepth
            );
        }

        void clearAll(GameContext* gameContext) {
            clear(true, true, true, gameContext);
        }

        void setViewPort(u32 viewportX, u32 viewportY, u32 viewportWidth, u32 viewportHeight, GameContext* gameContext) {
            float emuD3DView[4][4] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };

            // emulate the transformation applied by an equivalent Direct3D viewport
            emuD3DView[0][0] = viewportWidth / 640.0f;
            // no idea why this is necessary
            if (ff7DetectGameMode.getMode()->mode == FF7::GameModes::MODE_BATTLE) emuD3DView[1][1] = 1.0f;
            else emuD3DView[1][1] = viewportHeight / 480.0f;
            emuD3DView[3][0] = (((float)viewportX + (float)viewportWidth / 2.0f) - (float)640.0f / 2.0f) / ((float)640.0f / 2.0f);
            emuD3DView[3][1] = -(((float)viewportY + (float)viewportHeight / 2.0f) - (float)480.0f / 2.0f) / ((float)480.0f / 2.0f);

            ffRenderer.setRendererViewport(
                ffRenderer.getInternalCoordX(viewportX),
                ffWindow.getHeight() - ffRenderer.getInternalCoordY(viewportY + viewportHeight),
                ffRenderer.getInternalCoordX(viewportWidth),
                ffRenderer.getInternalCoordY(viewportHeight)
            );

            ffRenderer.setD3DViewMatrix(emuD3DView);
        }

        void setBackground(ColorBGRA* colorData, GameContext* gameContext) {
            ffDriverLog.write("%s: BGRA(%f, %f, %f, %f)", __func__, colorData->blue, colorData->green, colorData->red, colorData->alpha);

            ffRenderer.setClearColor(
                255 * colorData->red,
                255 * colorData->green,
                255 * colorData->blue
            );
        }

        u32 initPolygonSet(PolygonSet* polygonSet) {

            if (polygonSet)
                polygonSet->indexedPrimitives = (IndexedPrimitive**)calloc(polygonSet->numgroups, 4);

            return true;
        }

        u32 loadGroup(u32 groupNum, MatrixSet* matrixSet, AuxillaryGFX* auxGFX, PGroup* groupData, PFilePartHeader* polygonData, PolygonSet* polygon, GameContext* gameContext) {
            IndexedPrimitive* ip;
            AuxillaryGFX* hundred_data;
            PGroup* group_data;
            uint32_t numvert;
            uint32_t numpoly;
            uint32_t offvert;
            uint32_t offpoly;
            uint32_t offtex;
            uint32_t i;

            if (!polygonData) return false;
            if (!polygon->indexedPrimitives) return false;
            if (groupNum >= polygonData->numGroups) return false;

            ip = (IndexedPrimitive*)calloc(sizeof(*ip), 1);

            ip->primitiveType = NULL; // render using triangles by default
            ip->vertexSize = sizeof(nVertex);

            group_data = &polygonData->groups[groupNum];
            hundred_data = &polygonData->auxillaries[groupNum];
            numvert = group_data->vertexGroupLength;
            numpoly = group_data->polygonGroupLength;
            offvert = group_data->vertexGroupBaseOffset;
            offpoly = group_data->polygonGroupBaseOffset;
            offtex = group_data->TextureCoordsBaseOffset;

            ip->vertexCount = numvert;
            ip->indexCount = numpoly * 3;
            ip->vertices = (nVertex*)calloc(sizeof(*ip->vertices), ip->vertexCount);
            ip->indices = (u16*)calloc(sizeof(*ip->indices), ip->indexCount);

            for (i = 0; i < numvert; i++)
            {
                if (polygonData->vertices) memcpy(&ip->vertices[i]._, &polygonData->vertices[offvert + i], sizeof(ip->vertices[i]._));
                if (polygonData->vertexColorData) memcpy(&ip->vertices[i].color.color, &polygonData->vertexColorData[offvert + i], sizeof(ip->vertices[i].color.color));
                if (group_data->isTextured && polygonData->texCoords) memcpy(&ip->vertices[i].u, &polygonData->texCoords[offtex + i], sizeof(polygonData->texCoords[i]));

                if (hundred_data && (hundred_data->specialDamageFlags & FF7DRIVER_BIT_CALC(V_ALPHABLEND))) ip->vertices[i].color.a = hundred_data->vertexAlpha;
            }

            for (i = 0; i < numpoly; i++)
            {
                if (polygonData->polygons)
                {
                    GamePolygon* poly_data = &polygonData->polygons[offpoly + i];

                    ip->indices[i * 3] = poly_data->vertex1;
                    ip->indices[i * 3 + 1] = poly_data->vertex2;
                    ip->indices[i * 3 + 2] = poly_data->vertex3;
                }
            }

            polygon->indexedPrimitives[groupNum] = ip;

            return true;
        }

        void setMatrix(u32 unk, Matrix* matrix, MatrixSet* matrixSet, GameContext* gameContext) {
            switch (unk)
            {
            case 0:
                if (!matrixSet->matrixWorld) matrixSet->matrixWorld = matrix;
                else memcpy(matrixSet->matrixWorld, matrix, sizeof(*matrix));
                break;

            case 1:
                if (!matrixSet->matrixView) matrixSet->matrixView = matrix;
                else memcpy(matrixSet->matrixView, matrix, sizeof(*matrix));
                break;

            case 2:
                if (!matrixSet->matrixProjection) matrixSet->matrixProjection = matrix;
                else memcpy(matrixSet->matrixProjection, matrix, sizeof(*matrix));
                break;
            }
        }

        void unloadTexture(TextureSet* texture) {
            if (!texture) return;
            if (!texture->textureHandle) return;

            uint32_t texCount = texture->textureHandle[0];

            for (uint32_t idx = 1; idx < texCount; idx++)
                ffRenderer.destroyTexture(texture->textureHandle[idx]);

            ::free(texture->textureHandle);

            texture->textureHandle = 0;
        }

        TextureSet* loadTexture(TextureSet* texture, TextureHeader* textureHeader, TextureFormat* textureFormat) {
            Palette* palette = 0;
            bool color_key = false;

            textureFormat = &textureHeader->textureFormat;

            // no existing texture set, create one
            if (!texture) texture = createTextureSet();

            // texture handle array may not have been initialized
            if (!texture->textureHandle)
            {
                // allocate some more textures just in case, there could be more palettes we don't know about yet
                // we allocate +1 item, as the first one will contain the total number of items in the array
                u32 texCount = textureHeader->paletteCount > 0 ? (textureHeader->paletteCount * 2) : 1;

                texture->textureHandle = new u32[texCount + 1]{ bgfx::kInvalidHandle };
                texture->textureHandle[0] = texCount;
            }

            // number of palettes has changed, reload the texture completely
            if (texture->textureHandle[0] != (textureHeader->paletteCount * 2) && !(textureHeader->paletteCount == 0 && texture->textureHandle[0] == 1))
            {
                unloadTexture(texture);

                return loadTexture(texture, textureHeader, textureFormat);
            }

            // make sure the information in the texture set is consistent
            texture->texHeader = textureHeader;
            texture->textureFormat = textureFormat;

            // check if this is suppposed to be a framebuffer texture, we may not have to do anything
            if (textureHeader->version == FF7DRIVER_FFNX_TEX_VERSION) {
                ffDriverLog.write("%s: Framebuffer %s[%u]", __func__, textureHeader->file.pc_name, textureHeader->paletteIndex);

                ffRenderer.setRendererFramebuffer(
                    texture->texHeader->textureFormat.width,
                    texture->texHeader->textureFormat.height,
                    FFNx::RendererCommittedTextureType::BGRA
                );

                return texture;
            }

            // initialize palette index to a sane value if it hasn't been set
            if (textureHeader->paletteCount > 0) {
                if (texture->paletteIndex == -1)
                    textureHeader->paletteIndex = 0;
                else
                    textureHeader->paletteIndex = texture->paletteIndex;
            }
            else
                textureHeader->paletteIndex = 0;

            // create palette structure if it doesn't exist already
            if (textureHeader->paletteCount > 1 && texture->palette == 0)
                palette = createPaletteForTex(textureFormat->bitsperpixel, textureHeader, texture);

            if (textureFormat->palettes == 0) textureFormat->palettes = textureHeader->paletteEntries;

            // convert texture data from source format and load it
            if (textureFormat != NULL && textureHeader->imageData != NULL)
            {
                // the texture handle for the current palette is missing, convert & load it
                if (!texture->textureHandle[texture->paletteIndex + 1])
                {
                    uint32_t c = 0;
                    uint32_t w = textureHeader->version == FF7DRIVER_FFNX_TEX_VERSION ? textureHeader->fb_tex.w : textureFormat->width;
                    uint32_t h = textureHeader->version == FF7DRIVER_FFNX_TEX_VERSION ? textureHeader->fb_tex.h : textureFormat->height;
                    bool invert_alpha = false;
                    // pre-calculate some useful data for palette conversion
                    uint32_t palette_offset = textureHeader->paletteIndex * textureHeader->paletteEntries;
                    uint32_t reference_alpha = ((textureHeader->referenceAlpha & 0xFF) << 24);

                    // detect 16-bit PSX 5551 format with mask bit
                    if (textureFormat->bitsperpixel == 16 && textureFormat->alphaMask == 0x8000)
                    {
                        // correct incomplete texture format in FF7
                        textureFormat->blueMask = 0x001F;
                        textureFormat->greenMask = 0x03E0;
                        textureFormat->redMask = 0x7C00;
                        textureFormat->blueShift = 0;
                        textureFormat->greenShift = 5;
                        textureFormat->redShift = 10;
                        textureFormat->blueMax = 31;
                        textureFormat->greenMax = 31;
                        textureFormat->redMax = 31;

                        invert_alpha = true;
                    }

                    // check if this texture can be loaded from the modpath, we may not have to do any conversion
                    //TODO: LOAD EXTERNAL TEXTURE
                    //if (load_external_texture(VPTR(texture_set), VPTR(tex_header))) return VPTR(texture_set);

                    // find out if color keying is enabled for this texture
                    color_key = textureHeader->colorKey;

                    // find out if color keying is enabled for this particular palette
                    if (textureHeader->usePaletteColorKey)
                        color_key = textureHeader->paletteColorKey[textureHeader->paletteIndex];

                    // convert source data
                    uint32_t* converted_data = internalConvertImageData(textureHeader->imageData, w, h, textureFormat, invert_alpha, color_key, palette_offset, reference_alpha);

                    uint32_t texIdx = texture->paletteIndex + 1;

                    if (converted_data != NULL) {
                        if (texture->textureHandle[texIdx] != bgfx::kInvalidHandle)
                            ffRenderer.destroyTexture(texture->textureHandle[texIdx]);

                        texture->textureHandle[texIdx] = ffRenderer.commitTexture(
                            std::string(textureHeader->file.pc_name),
                            w,
                            h,
                            0,
                            0,
                            converted_data,
                            w * h * 4,
                            NULL,
                            false,
                            FFNx::RendererCommittedTextureType::BGRA,
                            currentTextureFilterFlags
                        );
                    }

                    ffDriverLog.write("%s: %s%u", __func__, textureHeader->file.pc_name, texture->textureHandle[texIdx]);
                }
            }
            else ffDriverLog.write("UNEXPECTED: no texture format specified or no source data\n");

            return texture;
        }

        u32 paletteChanged(u32 unk1, u32 unk2, u32 unk3, Palette* palette, TextureSet* texture) {
            if (palette == 0 || texture == 0) return false;

            // texture loader logic handles missing palettes, just make sure the new palette has been loaded
            texture = loadTexture(texture, texture->texHeader, texture->textureFormat);

            currentTexIndex = texture->textureHandle[texture->texHeader->paletteIndex + 1];

            return true;
        }

        u32 writePalette(u32 sourceOffset, u32 size, void* source, u32 destOffset, Palette* palette, TextureSet* texture) {
            if (palette == 0) return false;

            // FF7 writes to one palette at a time
            if ((size / texture->texHeader->paletteEntries) > 1) ffDriverLog.write("UNEXPECTED: multipalette write");

            // make sure the palette actually changed to avoid redundant texture reloads
            if (memcmp((uint32_t*)texture->texHeader->textureFormat.paletteData + destOffset, ((uint32_t*)source + sourceOffset), size * 4))
                memcpy((uint32_t*)texture->texHeader->textureFormat.paletteData + destOffset, ((uint32_t*)source + sourceOffset), size * 4);

            return true;
        }

        BlendMode* blendMode(u32 unk, GameContext* gameContext) {
            switch (unk)
            {
            case 0:
                return &blendModes[0];
            case 1:
                return &blendModes[1];
            case 2:
                return &blendModes[2];
            case 3:
                return &blendModes[3];
            case 4:
                ffDriverLog.write("UNEXPECTED: blend mode 4 requested");
                return &blendModes[4];
            }

            ffDriverLog.write("UNEXPECTED: invalid blend mode");

            return 0;
        }

        void field_64(u32 unk1, u32 unk2, GameContext* gameContext) {
            internalSetRenderState(unk1, unk2, gameContext);
        }

        void setRenderState(AuxillaryGFX* auxGFX, GameContext* gameContext) {
            uint32_t features;
            uint32_t options;

            if (auxGFX == 0) return;

            features = auxGFX->field_C;
            options = auxGFX->field_8;

            // helper macro to check if a bit is set
            // to be able to tell which bits we haven't handled, this macro will also clear
            // a bit after checking it, be extremely careful not to copy/paste any
            // invocation of this macro, the second invocation will not work!
#define FF7DRIVER_CHECK_BIT(X, Y) ((X) & FF7DRIVER_BIT_CALC((Y))) && (((X &= ~FF7DRIVER_BIT_CALC((Y))) || true))

            if (FF7DRIVER_CHECK_BIT(features, V_WIREFRAME)) internalSetRenderState(V_WIREFRAME, FF7DRIVER_CHECK_BIT(options, V_WIREFRAME), gameContext);

            if (FF7DRIVER_CHECK_BIT(features, V_TEXTURE)) {
                if (auxGFX->textureSet) {
                    currentTexName = auxGFX->textureSet->texHeader->file.pc_name;
                    currentTexIndex = auxGFX->textureSet->textureHandle[auxGFX->textureSet->texHeader->paletteIndex + 1];
                }
            }

            if (FF7DRIVER_CHECK_BIT(features, V_LINEARFILTER)) internalSetRenderState(V_LINEARFILTER, FF7DRIVER_CHECK_BIT(options, V_LINEARFILTER), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_PERSPECTIVE)) internalSetRenderState(V_PERSPECTIVE, FF7DRIVER_CHECK_BIT(options, V_PERSPECTIVE), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_COLORKEY)) internalSetRenderState(V_COLORKEY, FF7DRIVER_CHECK_BIT(options, V_COLORKEY), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_DITHER)) internalSetRenderState(V_DITHER, FF7DRIVER_CHECK_BIT(options, V_DITHER), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_ALPHABLEND))
            {
                if (FF7DRIVER_CHECK_BIT(options, V_ALPHABLEND))
                {
                    if (gameContext->field_93C)
                    {
                        if (gameContext->currentAuxillary)
                            internalSetBlendMode(FFNx::RendererBlendMode(auxGFX->blendMode));
                        else
                            internalSetBlendMode(FFNx::RendererBlendMode::BLEND_NONE);
                    }
                    else
                        internalSetBlendMode(FFNx::RendererBlendMode(auxGFX->blendMode));
                }
                else
                    internalSetBlendMode(FFNx::RendererBlendMode::BLEND_NONE);
            }
            if (FF7DRIVER_CHECK_BIT(features, V_ALPHATEST)) internalSetRenderState(V_ALPHATEST, FF7DRIVER_CHECK_BIT(options, V_ALPHATEST), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_CULLFACE)) internalSetRenderState(V_CULLFACE, FF7DRIVER_CHECK_BIT(options, V_CULLFACE), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_NOCULL)) internalSetRenderState(V_NOCULL, FF7DRIVER_CHECK_BIT(options, V_NOCULL), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_DEPTHTEST)) internalSetRenderState(V_DEPTHTEST, FF7DRIVER_CHECK_BIT(options, V_DEPTHTEST), gameContext);
            if (FF7DRIVER_CHECK_BIT(features, V_DEPTHMASK)) internalSetRenderState(V_DEPTHMASK, FF7DRIVER_CHECK_BIT(options, V_DEPTHMASK), gameContext);
            /*
            if (FF7DRIVER_CHECK_BIT(features, V_SHADEMODE))
            {
                if (FF7DRIVER_CHECK_BIT(options, V_SHADEMODE) && !gameContext->field_92C)
                {
                    switch (auxGFX->shadeMode) {
                    case 1:
                        glShadeModel(GL_FLAT);
                        break;
                    case 2:
                        glShadeModel(GL_SMOOTH);
                        break;
                    default:
                        ffDriverLog.write("GLITCH: missing shade mode %i\n", auxGFX->shadeMode);
                    }
                }
                else
                    glShadeModel(GL_FLAT);
            }*/
        }

        void field_74(u32 unk, GameContext* gameContext) {
            if (unk > 4) return;

            setRenderState(gameContext->auxillaries[unk], gameContext);
        }

        void field_78(PolygonSet* polygonSet, GameContext* gameContext) {
            MatrixSet* matrix_set;
            GraphicsChain* struc_49;
            AuxillaryGFX* hundred_data = 0;
            PGroup* group_data = 0;
            Matrix* matrix = 0;
            PolygonSetChain* struc_77;
            IndexedPrimitive* ip = 0;
            nVertex* vertices;
            DrawableObjectChain* struc_84;
            DrawableState* struc_186;
            uint32_t instance_type = -1;
            uint32_t group_counter = 0;
            bool instanced = false;
            bool correct_frame = false;
            uint32_t instance_transform_mode;
            Matrix tmp_matrix;
            bool trace_field_78 = false;
            Matrix* model_matrix = 0;

            if (!gameContext->in_scene) return;

            if (!polygonSet) return;

            if (!polygonSet->field_0) return;

            matrix_set = polygonSet->matrixSet;

            if (matrix_set) model_matrix = matrix_set->matrixView;

            struc_49 = &polygonSet->field_14;

            if (struc_49->field_0)
            {
                instanced = true;

                correct_frame = (struc_49->graphicsInstance->frameCounter == struc_49->frameCounter);

                instance_transform_mode = struc_49->field_8;
                struc_84 = struc_49->graphicsObjectChain;

                if (struc_84) instance_type = struc_84->specialDamageFlags;
            }

            if (polygonSet->drainedHP) hundred_data = polygonSet->auxillaries;
            if (polygonSet->pFilePolygons) group_data = polygonSet->pFilePolygons->groups;

            while (group_counter < polygonSet->numgroups)
            {
                bool defer = false;
                bool zsort = false;

                struc_84 = struc_49->graphicsObjectChain;

                if (polygonSet->indexedPrimitives)
                    ip = polygonSet->indexedPrimitives[group_counter];

                if (ip)
                    vertices = ip->vertices;

                if (polygonSet->hasPerGroupAuxillaies)
                    hundred_data = polygonSet->groupAuxillaries[group_counter];

                if (hundred_data)
                {
                    if (gameContext->field_91C && hundred_data->zSort)
                        zsort = true;
                    else if (!gameContext->field_928)
                        defer = (hundred_data->field_8 & (FF7DRIVER_BIT_CALC(V_ALPHABLEND) | FF7DRIVER_BIT_CALC(V_TMAPBLEND)));
                }

                if (!defer)
                    setRenderState(hundred_data, (GameContext*)gameContext);

                if (matrix_set && matrix_set->matrixProjection) {
                    ffRenderer.setD3DProjectionMatrix(matrix_set->matrixProjection->elements);
                }

                if (instanced)
                {
                    if (correct_frame)
                    {
                        while (struc_84)
                        {
                            if (instance_type == 2)
                            {
                                if (instance_transform_mode == 1)
                                    matrix = &struc_84->matrix;
                                else if (instance_transform_mode == 2)
                                {
                                    multiplyMatrix(&struc_84->matrix, gameContext->cameraMatrix, &tmp_matrix);
                                    matrix = &tmp_matrix;
                                }

                                if (matrix && matrix_set && !zsort) {
                                    ffRenderer.setWorldViewMatrix(matrix->elements);
                                }

                                struc_186 = struc_84->drawableState;

                                if (struc_186->polygontype == 0x11)
                                    vertices = struc_186->nvertexPointer;
                                else
                                    sub_671742(zsort, hundred_data, struc_186);

                                if (zsort)
                                    sub_665D9A(matrix, vertices, ip, hundred_data, struc_186, gameContext);
                                else
                                {
                                    internalRenderVertex(
                                        ip->primitiveType,
                                        vertices,
                                        ip->vertexCount,
                                        ip->indices,
                                        ip->indexCount,
                                        ip->vertexType == FF7_VERTEXTYPE_TLVERTEX,
                                        polygonSet->specialDamageFlags
                                    );
                                    // TODO: WANTS MIPMAP
                                }
                            }
                            else if (defer)
                            {
                                struc_77 = (PolygonSetChain*)sub_6A2865(gameContext->_3dobject_pool);

                                if (struc_77)
                                {
                                    struc_77->currentGroup = group_counter;
                                    struc_77->polygonSet = polygonSet;
                                    struc_77->auxillary = hundred_data;

                                    if (polygonSet->hasPaletteAuxillary)
                                        memcpy(&struc_77->paletteAuxillary, polygonSet->paletteAuxillary, sizeof(*polygonSet->paletteAuxillary));

                                    struc_77->useMatrix = 0;
                                    struc_77->use_matrix_pointer = 0;

                                    if (instance_transform_mode == 1)
                                    {
                                        struc_77->use_matrix_pointer = 1;
                                        struc_77->matrix_pointer = &struc_84->matrix;
                                    }

                                    else if (instance_transform_mode == 2)
                                    {
                                        struc_77->useMatrix = 1;
                                        multiplyMatrix(&struc_84->matrix, gameContext->cameraMatrix, &struc_77->matrix);
                                        matrix3x4(&struc_77->matrix);
                                    }
                                }
                            }
                            else if (instance_type == 0)
                            {
                                if (instance_transform_mode == 1) matrix = &struc_84->matrix;
                                else if (instance_transform_mode == 2)
                                {
                                    multiplyMatrix(&struc_84->matrix, gameContext->cameraMatrix, &tmp_matrix);
                                    matrix = &tmp_matrix;
                                }

                                if (ip)
                                {
                                    if (zsort) sub_665793(matrix, 0, ip, polygonSet, hundred_data, group_data, gameContext);
                                    else
                                    {
                                        if (matrix && matrix_set)
                                            ffRenderer.setWorldViewMatrix(matrix->elements);

                                        internalRenderVertex(
                                            ip->primitiveType,
                                            vertices,
                                            ip->vertexCount,
                                            ip->indices,
                                            ip->indexCount,
                                            ip->vertexType == FF7_VERTEXTYPE_TLVERTEX,
                                            polygonSet->specialDamageFlags
                                        );
                                        // TODO: WANTS MIPMAP
                                        // TODO: RE-DO WITH LIGHTNING SUPPORT AND MODEL MATRIX (model_matrix) SUPPORT
                                    }
                                }
                            }
                            else if (instance_type == 1)
                            {
                                if (instance_transform_mode == 1) matrix = &struc_84->matrix;
                                else if (instance_transform_mode == 2)
                                {
                                    multiplyMatrix(&struc_84->matrix, gameContext->cameraMatrix, &tmp_matrix);
                                    matrix = &tmp_matrix;
                                }

                                if (ip)
                                {
                                    if (struc_84->paletteAuxillary.addOffsets || struc_84->paletteAuxillary.field_7)
                                    {
                                        sub_68D2B8(group_counter, polygonSet, &struc_84->paletteAuxillary);
                                        sub_6B27A9(matrix, ip, polygonSet, hundred_data, group_data, &struc_84->paletteAuxillary, gameContext);
                                    }
                                    else
                                    {
                                        if (zsort)
                                            sub_665793(matrix, 0, ip, polygonSet, hundred_data, group_data, gameContext);
                                        else
                                        {
                                            sub_68D2B8(group_counter, polygonSet, &struc_84->paletteAuxillary);

                                            if (matrix && matrix_set)
                                                ffRenderer.setWorldViewMatrix(matrix->elements);

                                            internalRenderVertex(
                                                ip->primitiveType,
                                                vertices,
                                                ip->vertexCount,
                                                ip->indices,
                                                ip->indexCount,
                                                ip->vertexType == FF7_VERTEXTYPE_TLVERTEX,
                                                polygonSet->specialDamageFlags
                                            );
                                            // TODO: WANTS MIPMAP
                                            // TODO: RE-DO WITH LIGHTNING SUPPORT AND MODEL MATRIX (model_matrix) SUPPORT
                                        }
                                    }
                                }
                            }

                            struc_84 = (DrawableObjectChain*)struc_84->nextDrawable;
                        }
                    }
                }
                else
                {
                    if (defer)
                    {
                        struc_77 = (PolygonSetChain*)sub_6A2865(gameContext->_3dobject_pool);

                        struc_77->currentGroup = group_counter;
                        struc_77->polygonSet = polygonSet;
                        struc_77->auxillary = hundred_data;

                        if (polygonSet->hasPaletteAuxillary)
                            memcpy(&struc_77->paletteAuxillary, polygonSet->paletteAuxillary, sizeof(*polygonSet->paletteAuxillary));

                        struc_77->useMatrix = 0;
                        struc_77->use_matrix_pointer = 0;

                        if (matrix_set && matrix_set->matrixWorld)
                        {
                            struc_77->useMatrix = 1;
                            memcpy(&struc_77->matrix, matrix_set->matrixWorld, sizeof(*matrix_set->matrixWorld));
                        }
                    }
                    else
                    {
                        if (ip)
                        {
                            if (zsort)
                                sub_665793(matrix_set->matrixWorld, 0, ip, polygonSet, hundred_data, group_data, gameContext);
                            else
                            {
                                if (matrix_set)
                                    ffRenderer.setWorldViewMatrix(matrix_set->matrixWorld->elements);

                                internalRenderVertex(
                                    ip->primitiveType,
                                    vertices,
                                    ip->vertexCount,
                                    ip->indices,
                                    ip->indexCount,
                                    ip->vertexType == FF7_VERTEXTYPE_TLVERTEX,
                                    polygonSet->specialDamageFlags
                                );
                                // TODO: WANTS MIPMAP
                                // TODO: RE-DO WITH LIGHTNING SUPPORT AND MODEL MATRIX (model_matrix) SUPPORT
                            }
                        }
                    }
                }

                if (hundred_data) hundred_data = &hundred_data[1];
                if (group_data) group_data = &group_data[1];

                group_counter++;
            }
        }

        void deferredDraw(PolygonSetChain* polygonSetChain, GameContext* gameContext) {
            IndexedPrimitive* ip;
            Matrix* model_matrix = 0;

            if (!polygonSetChain->polygonSet->indexedPrimitives) return;

            ip = polygonSetChain->polygonSet->indexedPrimitives[polygonSetChain->currentGroup];

            if (!ip) return;

            setRenderState(polygonSetChain->auxillary, gameContext);

            if (polygonSetChain->useMatrix) {
                ffRenderer.setWorldViewMatrix(polygonSetChain->matrix.elements);
            }

            if (polygonSetChain->use_matrix_pointer) {
                ffRenderer.setWorldViewMatrix(polygonSetChain->matrix_pointer->elements);
            }

            if (polygonSetChain->polygonSet->matrixSet) {
                model_matrix = polygonSetChain->polygonSet->matrixSet->matrixView;
            }

            internalRenderVertex(
                NULL,
                ip->vertices,
                ip->vertexCount,
                ip->indices,
                ip->indexCount,
                ip->vertexType == FF7_VERTEXTYPE_TLVERTEX,
                polygonSetChain->polygonSet->specialDamageFlags
            );
        }

        void field_80(GraphicsObject* graphicsObj, GameContext* gameContext) {
            if (!graphicsObj) return;

            field_78(graphicsObj->polygonSet, gameContext);
        }

        void field_84(u32 unk, GameContext* gameContext) {
            if (!gameContext->in_scene) return;

            gameContext->field_928 = unk;

            if (!unk)
            {
                gameContext->polygonSet2EC->field_0 = true;
                gameContext->polygonSet2F0->field_0 = false;
                field_78(gameContext->polygonSet2EC, gameContext);
            }
            else
            {
                gameContext->polygonSet2EC->field_0 = false;
                gameContext->polygonSet2F0->field_0 = true;
                field_78(gameContext->polygonSet2F0, gameContext);
            }
        }

        void field_90(u32 unk) {
            // Not implemented
        }

        void setRenderState_flat2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            if (!polygonSet->drainedHP) return;

            setRenderState(polygonSet->auxillaries, gameContext);
        }

        void setRenderState_smooth2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_textured2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_paletted2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void drawFlat2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            genericDraw(polygonSet, indexVert, gameContext, FF7_VERTEXTYPE_TLVERTEX);
        }

        void drawSmooth2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            drawFlat2D(polygonSet, indexVert, gameContext);
        }

        void drawTextured2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            drawFlat2D(polygonSet, indexVert, gameContext);
        }

        void drawPaletted2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            genericDrawPaletted(polygonSet, indexVert, gameContext, FF7_VERTEXTYPE_TLVERTEX);
        }

        void setRenderState_flat3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            if (!polygonSet->drainedHP) return;

            setRenderState(polygonSet->auxillaries, gameContext);

            if (indexVert->graphicsObject->useMatrixPointer) {
                ffRenderer.setWorldViewMatrix(indexVert->graphicsObject->matrixPtr->elements);
            }
            else {
                ffRenderer.setWorldViewMatrix(indexVert->graphicsObject->matrix.elements);
            }
        }

        void setRenderState_smooth3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_textured3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_paletted3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat3D(polygonSet, indexVert, gameContext);
        }

        void drawFlat3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            genericDraw(polygonSet, indexVert, gameContext, FF7_VERTEXTYPE_LVERTEX);
        }

        void drawSmooth3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            drawFlat3D(polygonSet, indexVert, gameContext);
        }

        void drawTextured3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            drawFlat3D(polygonSet, indexVert, gameContext);
        }

        void drawPaletted3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            genericDrawPaletted(polygonSet, indexVert, gameContext, FF7_VERTEXTYPE_LVERTEX);
        }

        void setRenderState_flatlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_smoothlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void draw_flatlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            internalRenderVertex(
                BGFX_STATE_PT_LINES,
                indexVert->vertices,
                indexVert->vertexCount,
                indexVert->indices,
                indexVert->indexCount,
                true,
                polygonSet->specialDamageFlags
            );
            // TODO: WANTS MIPMAP
        }

        void draw_smoothlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            draw_flatlines(polygonSet, indexVert, gameContext);
        }

        void field_EC(GameContext* gameContext) {
            // Not implemented
        }

        void drawSingleTriangle(nVertex* vertices) {
            WORD indices[] = { 0, 1, 2 };

            internalRenderVertex(
                NULL,
                vertices,
                3,
                indices,
                3,
                true, // FF7_VERTEXTYPE_TLVERTEX
                true
            );
        }

        void sub_6B2720(IndexedPrimitive* ip)
        {
            internalRenderVertex(
                NULL,
                ip->vertices,
                ip->vertexCount,
                ip->indices,
                ip->indexCount,
                true, // FF7_VERTEXTYPE_TLVERTEX
                true
            );
        }

        void destroyD3D2IndexedPrimitive(IndexedPrimitive* ip)
        {
            if (!ip) return;

            if (ip->vertices) free(ip->vertices);
            if (ip->indices) free(ip->indices);

            free(ip);
        }
    };

}

#endif
