#ifndef __FF7DRIVER_DRIVER_HPP__
#define __FF7DRIVER_DRIVER_HPP__

#include "ff7/main.hpp"

namespace FF7 {

    class Driver {
    private:
        // Private context handle
        GameContext* gameContext;

        // Original game resolution
        uint32_t gameWidth = 0;
        uint32_t gameHeight = 0;

        // Driver callbacks
        GraphicsDriverCallbacks* engineCallbacks;

        void replaceCoreFunctions() {
            // Prevent the game to crash when closed
            ffPatch.replaceFunction(FF7_FN_SUB_6A2631, &FF7::GenericCallbacks::sub_6A2631);
            ffPatch.replaceFunction(ff7DriverMusic.getEnginePointer_CleanupMidi(), &FF7::GenericCallbacks::noop);

            /* MIDI */
            ffPatch.replaceFunction(FF7_FN_MIDI_INIT, &FF7::MidiCallbacks::midiInit);
            ffPatch.replaceFunction(FF7_FN_PLAY_MIDI, &FF7::MidiCallbacks::playMidi);
            ffPatch.replaceFunction(FF7_FN_CROSS_FADE_MIDI, &FF7::MidiCallbacks::crossFadeMidi);
            ffPatch.replaceFunction(FF7_FN_PAUSE_MIDI, &FF7::MidiCallbacks::pauseMidi);
            ffPatch.replaceFunction(FF7_FN_RESTART_MIDI, &FF7::MidiCallbacks::restartMidi);
            ffPatch.replaceFunction(FF7_FN_STOP_MIDI, &FF7::MidiCallbacks::stopMidi);
            ffPatch.replaceFunction(FF7_FN_MIDI_STATUS, &FF7::MidiCallbacks::midiStatus);
            ffPatch.replaceFunction(FF7_FN_SET_MASTER_MIDI_VOLUME, &FF7::MidiCallbacks::setMasterMidiVolume);
            ffPatch.replaceFunction(FF7_FN_SET_MIDI_VOLUME, &FF7::MidiCallbacks::setMidiVolume);
            ffPatch.replaceFunction(FF7_FN_SET_MIDI_VOLUME_TRANS, &FF7::MidiCallbacks::setMidiVolumeTrans);
            ffPatch.replaceFunction(FF7_FN_SET_MIDI_TEMPO, &FF7::MidiCallbacks::setMidiTempo);

            /* Movies */
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_PrepareMovie(), &FF7::MovieCallbacks::prepareMovie);
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_ReleaseMovieObjects(), &FF7::MovieCallbacks::releaseMovieObjects);
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_StartMovie(), &FF7::MovieCallbacks::startMovie);
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_UpdateMovieSample(), &FF7::MovieCallbacks::updateMovieSample);
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_StopMovie(), &FF7::MovieCallbacks::stopMovie);
            ffPatch.replaceFunction(ff7DriverMovie.getEnginePointer_GetMovieFrame(), &FF7::MovieCallbacks::getMovieFrame);

            // D3D required patches
            FFNx::Memory::memsetCode(FF7_FN_SUB_6B27A9 + 25, 0x90, 6);
            ffPatch.replaceFunction(FF7_FN_SUB_6B26C0, &FF7::EngineCallbacks::drawSingleTriangle);
            ffPatch.replaceFunction(FF7_FN_SUB_6B2720, &FF7::EngineCallbacks::sub_6B2720);

            // replace framebuffer access routine with our own version
            ffPatch.replaceFunction(FF7_FN_DESTROYD3D2IP, &FF7::EngineCallbacks::destroyD3D2IndexedPrimitive);

            // Texture caching required patches
            ffPatch.replaceFunction(FF7_FN_SUB_673F5C, &FF7::FileManagerCallbacks::sub_673F5C);
            ffPatch.replaceFunction(FF7_FN_LOAD_TEX_FILE, &FF7::FileManagerCallbacks::loadTextureFile);
            ffPatch.replaceFunction(FF7_FN_DESTROY_TEX_HEADER, &FF7::FileManagerCallbacks::destroyTextureHeader);
            ffPatch.replaceFunction(FF7_FN_OPEN_FILE, &FF7::FileManagerCallbacks::openFile);
            ffPatch.replaceFunction(FF7_FN_CLOSE_FILE, &FF7::FileManagerCallbacks::closeFile);
            ffPatch.replaceFunction(FF7_FN_READ_FILE, &FF7::FileManagerCallbacks::readFile);
            ffPatch.replaceFunction(FF7_FN___READ_FILE, &FF7::FileManagerCallbacks::__readFile);
            ffPatch.replaceFunction(FF7_FN___READ, &FF7::FileManagerCallbacks::__read);
            ffPatch.replaceFunction(FF7_FN_WRITE_FILE, &FF7::FileManagerCallbacks::writeFile);
            ffPatch.replaceFunction(FF7_FN_GET_FILESIZE, &FF7::FileManagerCallbacks::getFilesize);
            ffPatch.replaceFunction(FF7_FN_TELL_FILE, &FF7::FileManagerCallbacks::tellFile);
            ffPatch.replaceFunction(FF7_FN_SEEK_FILE, &FF7::FileManagerCallbacks::seekFile);
            ffPatch.replaceFunction(FF7_FN_OPEN_LGP_FILE, &FF7::FileManagerCallbacks::openLgpFile);
            ffPatch.replaceFunction(FF7_FN_LGP_CHDIR, &FF7::FileManagerCallbacks::lgpChdir);
            ffPatch.replaceFunction(FF7_FN_LGP_OPEN_FILE, &FF7::FileManagerCallbacks::lgpOpenFile);
            ffPatch.replaceFunction(FF7_FN_LGP_SEEK_FILE, &FF7::FileManagerCallbacks::lgpSeekFile);
            ffPatch.replaceFunction(FF7_FN_LGP_READ, &FF7::FileManagerCallbacks::lgpRead);
            ffPatch.replaceFunction(FF7_FN_LGP_GET_FILESIZE, &FF7::FileManagerCallbacks::lgpGetFilesize);
            ffPatch.replaceFunction(FF7_FN_LGP_READ_FILE, &FF7::FileManagerCallbacks::lgpReadFile);

            // P file required patches
            ffPatch.replaceFunction(FF7_FN_LOAD_P_FILE, &FF7::FileManagerCallbacks::loadPFile);

            // DirectInput hack, try to reacquire on any error
            FFNx::Memory::memsetCode(FF7_FN_DINPUT_GETDATA2 + 0x65, 0x90, 9);
            FFNx::Memory::memsetCode(FF7_FN_DINPUT_GETSTATE2 + 0x3C, 0x90, 9);
            FFNx::Memory::memsetCode(FF7_FN_DINPUT_GETSTATE2 + 0x7E, 0x90, 5);
            FFNx::Memory::memsetCode(FF7_FN_DINPUT_ACQUIRE_KEYBOARD + 0x31, 0x90, 5);

            // Do not hook mouse
            ffPatch.replaceFunction(FF7_FN_DINPUT_CREATEDEVICE_MOUSE, &FF7::GenericCallbacks::noop);
        }

        void setCallbacks() {
            /* ENGINE */
            engineCallbacks = (GraphicsDriverCallbacks*)calloc(1, sizeof(GraphicsDriverCallbacks));

            engineCallbacks->createDriver = FF7::EngineCallbacks::createDriver;
            engineCallbacks->destroyDriver = FF7::EngineCallbacks::destroyDriver;
            engineCallbacks->lock = FF7::EngineCallbacks::lock;
            engineCallbacks->unlock = FF7::EngineCallbacks::unlock;
            engineCallbacks->flip = FF7::EngineCallbacks::flip;
            engineCallbacks->clear = FF7::EngineCallbacks::clear;
            engineCallbacks->clearAll = FF7::EngineCallbacks::clearAll;
            engineCallbacks->setViewPort = FF7::EngineCallbacks::setViewPort;
            engineCallbacks->setBackGround = FF7::EngineCallbacks::setBackground;
            engineCallbacks->initPolygonSet = FF7::EngineCallbacks::initPolygonSet;
            engineCallbacks->loadGroup = FF7::EngineCallbacks::loadGroup;
            engineCallbacks->setMatrix = FF7::EngineCallbacks::setMatrix;
            engineCallbacks->unloadTexture = FF7::EngineCallbacks::unloadTexture;
            engineCallbacks->loadTexture = FF7::EngineCallbacks::loadTexture;
            engineCallbacks->paletteChanged = FF7::EngineCallbacks::paletteChanged;
            engineCallbacks->writePalette = FF7::EngineCallbacks::writePalette;
            engineCallbacks->blendmode = FF7::EngineCallbacks::blendMode;
            engineCallbacks->lightPolygonSet = gameContext->externals->genericLightPolygonSet;
            engineCallbacks->field_64 = FF7::EngineCallbacks::field_64;
            engineCallbacks->setRenderState = FF7::EngineCallbacks::setRenderState;
            engineCallbacks->_setRenderState = FF7::EngineCallbacks::_setRenderState;
            engineCallbacks->__setRenderState = FF7::EngineCallbacks::__setRenderState;
            engineCallbacks->field_74 = FF7::EngineCallbacks::field_74;
            engineCallbacks->field_78 = FF7::EngineCallbacks::field_78;
            engineCallbacks->deferredDraw = FF7::EngineCallbacks::deferredDraw;
            engineCallbacks->field_80 = FF7::EngineCallbacks::field_80;
            engineCallbacks->field_84 = FF7::EngineCallbacks::field_84;
            engineCallbacks->beginScene = FF7::EngineCallbacks::beginScene;
            engineCallbacks->endScene = FF7::EngineCallbacks::endScene;
            engineCallbacks->field_90 = FF7::EngineCallbacks::field_90;
            engineCallbacks->setrenderstate_flat2D = FF7::EngineCallbacks::setRenderState_flat2D;
            engineCallbacks->setrenderstate_smooth2D = FF7::EngineCallbacks::setRenderState_smooth2D;
            engineCallbacks->setrenderstate_textured2D = FF7::EngineCallbacks::setRenderState_textured2D;
            engineCallbacks->setrenderstate_paletted2D = FF7::EngineCallbacks::setRenderState_paletted2D;
            engineCallbacks->_setrenderstate_paletted2D = FF7::EngineCallbacks::_setRenderState_paletted2D;
            engineCallbacks->drawFlat2D = FF7::EngineCallbacks::drawFlat2D;
            engineCallbacks->drawSmooth2D = FF7::EngineCallbacks::drawSmooth2D;
            engineCallbacks->drawTextured2D = FF7::EngineCallbacks::drawTextured2D;
            engineCallbacks->drawPaletted2D = FF7::EngineCallbacks::drawPaletted2D;
            engineCallbacks->setRenderState_flat3D = FF7::EngineCallbacks::setRenderState_flat3D;
            engineCallbacks->setRenderState_smooth3D = FF7::EngineCallbacks::setRenderState_smooth3D;
            engineCallbacks->setRenderState_textured3D = FF7::EngineCallbacks::setRenderState_textured3D;
            engineCallbacks->setRenderState_paletted3D = FF7::EngineCallbacks::setRenderState_paletted3D;
            engineCallbacks->_setRenderState_paletted3D = FF7::EngineCallbacks::_setRenderState_paletted3D;
            engineCallbacks->drawFlat3D = FF7::EngineCallbacks::drawFlat3D;
            engineCallbacks->drawSmooth3D = FF7::EngineCallbacks::drawSmooth3D;
            engineCallbacks->drawTextured3D = FF7::EngineCallbacks::drawTextured3D;
            engineCallbacks->drawPaletted3D = FF7::EngineCallbacks::drawPaletted3D;
            engineCallbacks->setrenderstate_flatlines = FF7::EngineCallbacks::setRenderState_flatlines;
            engineCallbacks->setrenderstate_smoothlines = FF7::EngineCallbacks::setRenderState_smoothlines;
            engineCallbacks->draw_flatlines = FF7::EngineCallbacks::draw_flatlines;
            engineCallbacks->draw_smoothlines = FF7::EngineCallbacks::draw_smoothlines;
            engineCallbacks->field_EC = FF7::EngineCallbacks::field_EC;
        }

        /* INTERNAL USAGE ONLY */
        static void _ff7driver_fn_debug(const char* str) {
            ffDriverLog.write("ff7.exe: %s", str);
        }

    public:
        Driver() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        ~Driver(){
            ffDriverLog.write("Destroying %s", __func__);
        }

        GraphicsDriverCallbacks* init(GameContext* givenContext) {
            // Store the context for further usage
            gameContext = givenContext;

            // Store original game resolution
            gameWidth = givenContext->resWidth;
            gameHeight = givenContext->resHeight;

            // Hook internal game debug function
            ffPatch.replaceFunction(FF7_FN_PRINT_DEBUG_STRING, &_ff7driver_fn_debug);

            // Initialize Window if requested
            if (!ffUserConfig.wantsFullscreen()) {
                ffWindow.setMetaInfo(FFNx::WindowMeta::TITLE, "Final Fantasy VII (powered by " _DLLOUTNAME ")");
                ffWindow.setMetaInfo(FFNx::WindowMeta::BACKEND, ffUserConfig.getBackend());
                ffWindow.init();
                gameContext->hwnd = ffWindow.getHWnd();
            }

            // Patch in-game functions
            replaceCoreFunctions();

            // Hook into game engine
            setCallbacks();

            // Set internals
            ff7DriverMovie.setGameContext(gameContext);

            // Initialize game mode detection
            ff7DetectGameMode.init(gameContext);

            // Initialize game file manager
            ff7FileManager.init(gameContext);

            return engineCallbacks;
        }
    };

}

#endif
