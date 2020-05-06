#ifndef __FF7DRIVER_CALLBACKS_HPP__
#define __FF7DRIVER_CALLBACKS_HPP__

#include "main.hpp"

namespace FF7 {

    namespace GenericCallbacks {
        void noop() {}

        /* Internals - used via memory patch */
        void sub_6A2631() {
            ffDriverLog.write("Calling function %s", __func__);

            noop();
        }
    }

    namespace FileManagerCallbacks {
        TextureHeader* sub_673F5C(FF7Struc91* struc91) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.sub_673F5C(struc91);
        }

        TextureHeader* loadTextureFile(FF7FileContext* fileContext, char* fileName)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.loadTextureFile(fileContext, fileName);
        }

        void destroyTextureHeader(TextureHeader* texHeader)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7FileManager.destroyTextureHeader(texHeader);
        }

        // ---

        PFilePartHeader* loadPFile(FF7FileContext* fileContext, bool createLists, char* fileName)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.loadPFile(fileContext, createLists, fileName);
        }

        // ---

        bool lgpChdir(char* path)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpChdir(path);
        }

        FILE* openLgpFile(char* fileName, uint32_t mode)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.openLgpFile(fileName, mode);
        }

        FF7LgpFile* lgpOpenFile(char* fileName, uint32_t lgpNum)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpOpenFile(fileName, lgpNum);
        }

        bool lgpSeekFile(uint32_t offset, uint32_t lgpNum)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpSeekFile(offset, lgpNum);
        }

        uint32_t lgpRead(uint32_t lgpNum, char* dest, uint32_t size)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpRead(lgpNum, dest, size);
        }

        uint32_t lgpReadFile(FF7LgpFile* file, uint32_t lgpNum, char* dest, uint32_t size)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpReadFile(file, lgpNum, dest, size);
        }

        uint32_t lgpGetFilesize(FF7LgpFile* file, uint32_t lgpNum)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.lgpGetFilesize(file, lgpNum);
        }

        // ---

        void closeFile(FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7FileManager.closeFile(file);
        }

        FF7File* openFile(FF7FileContext* fileContext, char* fileName)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.openFile(fileContext, fileName);
        }

        uint32_t __readFile(uint32_t count, char* buffer, FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.__readFile(count, buffer, file);
        }

        bool readFile(uint32_t count, void* buffer, FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.readFile(count, buffer, file);
        }

        uint32_t __read(FILE* file, void* buffer, uint32_t count)
        {
            //ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.__read(file, buffer, count);
        }

        bool writeFile(uint32_t count, void* buffer, FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.writeFile(count, buffer, file);
        }

        uint32_t getFilesize(FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.getFilesize(file);
        }

        uint32_t tellFile(FF7File* file)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7FileManager.tellFile(file);
        }

        void seekFile(FF7File* file, uint32_t offset)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7FileManager.seekFile(file, offset);
        }
    }

    namespace EngineCallbacks {

        u32 createDriver(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.createDriver(gameContext);
        }

        void destroyDriver(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.destroyDriver(gameContext);
        }

        u32 lock(u32 surface) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.lock(surface);
        }

        u32 unlock(u32 surface) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.unlock(surface);
        }

        void flip(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.flip(gameContext);
        }

        void clear(u32 clearColor, u32 clearDepth, u32 unk, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.clear(clearColor, clearDepth, unk, gameContext);
        }

        void clearAll(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.clearAll(gameContext);
        }

        void setViewPort(u32 viewportX, u32 viewportY, u32 viewportWidth, u32 viewportHeight, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setViewPort(viewportX, viewportY, viewportWidth, viewportHeight, gameContext);
        }

        void setBackground(ColorBGRA* colorData, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setBackground(colorData, gameContext);
        }

        u32 initPolygonSet(PolygonSet* polygonSet) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.initPolygonSet(polygonSet);
        }

        u32 loadGroup(u32 groupNum, MatrixSet* matrixSet, AuxillaryGFX* auxGFX, PGroup* groupData, PFilePartHeader* polygonData, PolygonSet* polygon, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.loadGroup(groupNum, matrixSet, auxGFX, groupData, polygonData, polygon, gameContext);
        }

        void setMatrix(u32 unk, Matrix* matrix, MatrixSet* matrixSet, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setMatrix(unk, matrix, matrixSet, gameContext);
        }

        void unloadTexture(TextureSet* texture) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.unloadTexture(texture);
        }

        TextureSet* loadTexture(TextureSet* texture, TextureHeader* textureHeader, TextureFormat* textureFormat) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.loadTexture(texture, textureHeader, textureFormat);
        }

        u32 paletteChanged(u32 unk1, u32 unk2, u32 unk3, Palette* palette, TextureSet* texture) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.paletteChanged(unk1, unk2, unk3, palette, texture);
        }

        u32 writePalette(u32 sourceOffset, u32 size, void* source, u32 destOffset, Palette* palette, TextureSet* texture) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.writePalette(sourceOffset, size, source, destOffset, palette, texture);
        }

        BlendMode* blendMode(u32 unk, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.blendMode(unk, gameContext);
        }

        void field_64(u32 unk1, u32 unk2, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_64(unk1, unk2, gameContext);
        }

        void setRenderState(AuxillaryGFX* auxGFX, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState(auxGFX, gameContext);
        }

        void _setRenderState(AuxillaryGFX* auxGFX, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState(auxGFX, gameContext);
        }

        void __setRenderState(AuxillaryGFX* auxGFX, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState(auxGFX, gameContext);
        }

        void field_74(u32 unk, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_74(unk, gameContext);
        }

        void field_78(PolygonSet* polygonSet, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_78(polygonSet, gameContext);
        }

        void deferredDraw(PolygonSetChain* polygonSetChain, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.deferredDraw(polygonSetChain, gameContext);
        }

        void field_80(GraphicsObject* graphicsObj, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_80(graphicsObj, gameContext);
        }

        void field_84(u32 unk, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_84(unk, gameContext);
        }

        u32 beginScene(u32 unk, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverEngine.beginScene(unk, gameContext);
        }

        void endScene(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.endScene(gameContext);
        }

        void field_90(u32 unk) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_90(unk);
        }

        void setRenderState_flat2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_flat2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_smooth2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_smooth2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_textured2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_textured2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_paletted2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_paletted2D(polygonSet, indexVert, gameContext);
        }

        void _setRenderState_paletted2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_paletted2D(polygonSet, indexVert, gameContext);
        }

        void drawFlat2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawFlat2D(polygonSet, indexVert, gameContext);
        }

        void drawSmooth2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawSmooth2D(polygonSet, indexVert, gameContext);
        }

        void drawTextured2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawTextured2D(polygonSet, indexVert, gameContext);
        }

        void drawPaletted2D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawPaletted2D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_flat3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_flat3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_smooth3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_smooth3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_textured3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_textured3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_paletted3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_paletted3D(polygonSet, indexVert, gameContext);
        }

        void _setRenderState_paletted3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_paletted3D(polygonSet, indexVert, gameContext);
        }

        void drawFlat3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawFlat3D(polygonSet, indexVert, gameContext);
        }

        void drawSmooth3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawSmooth3D(polygonSet, indexVert, gameContext);
        }

        void drawTextured3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawTextured3D(polygonSet, indexVert, gameContext);
        }

        void drawPaletted3D(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawPaletted3D(polygonSet, indexVert, gameContext);
        }

        void setRenderState_flatlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_flatlines(polygonSet, indexVert, gameContext);
        }

        void setRenderState_smoothlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.setRenderState_smoothlines(polygonSet, indexVert, gameContext);
        }

        void draw_flatlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.draw_flatlines(polygonSet, indexVert, gameContext);
        }

        void draw_smoothlines(PolygonSet* polygonSet, IndexedVertices* indexVert, GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.draw_smoothlines(polygonSet, indexVert, gameContext);
        }

        void field_EC(GameContext* gameContext) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.field_EC(gameContext);
        }

        void drawSingleTriangle(nVertex* vertices) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.drawSingleTriangle(vertices);
        }

        void sub_6B2720(IndexedPrimitive* ip) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.sub_6B2720(ip);
        }

        void destroyD3D2IndexedPrimitive(IndexedPrimitive* ip) {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverEngine.destroyD3D2IndexedPrimitive(ip);
        }
    };

    namespace MidiCallbacks {
        bool midiInit(uint32_t unk)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMusic.midiInit(unk);
        }

        void playMidi(uint32_t midi)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.playMidi(midi);
        }

        void crossFadeMidi(uint32_t midi, uint32_t time)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.crossFadeMidi(midi, time);
        }

        void pauseMidi()
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.pauseMidi();
        }

        void restartMidi()
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.restartMidi();
        }

        void stopMidi()
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.stopMidi();
        }

        bool midiStatus()
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMusic.midiStatus();
        }

        void setMasterMidiVolume(uint32_t volume)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.setMasterMidiVolume(volume);
        }

        void setMidiVolume(uint32_t volume)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.setMidiVolume(volume);
        }

        void setMidiVolumeTrans(uint32_t volume, uint32_t step)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.setMidiVolumeTrans(volume, step);
        }

        void setMidiTempo(uint8_t tempo)
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMusic.setMidiTempo(tempo);
        }

    }

    namespace MovieCallbacks {

        bool prepareMovie(char* name, uint32_t loop, struct DDDEVICE** dddevice, uint32_t dd2interface)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMovie.prepareMovie(name, loop, dddevice, dd2interface);
        }

        void releaseMovieObjects()
        {
            ffDriverLog.write("Calling function %s", __func__);

            ff7DriverMovie.releaseMovieObjects();
        }

        bool startMovie()
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMovie.startMovie();
        }

        bool stopMovie()
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMovie.stopMovie();
        }

        bool updateMovieSample(LPDIRECTDRAWSURFACE surface)
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMovie.updateMovieSample(surface);
        }

        uint32_t getMovieFrame()
        {
            ffDriverLog.write("Calling function %s", __func__);

            return ff7DriverMovie.getMovieFrame();
        }

    }

}

#endif
