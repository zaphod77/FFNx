#ifndef __FF7DRIVER_MODE_HPP__
#define __FF7DRIVER_MODE_HPP__

#include "ff7/main.hpp"

#define FF7DRIVER_NUM_MODES 28

namespace FF7 {

    enum GameModes
    {
        MODE_FIELD = 1,
        MODE_BATTLE,
        MODE_WORLDMAP,
        MODE_UNKNOWN4,
        MODE_MENU,
        MODE_HIGHWAY,
        MODE_CHOCOBO,
        MODE_SNOWBOARD,
        MODE_CONDOR,
        MODE_SUBMARINE,
        MODE_COASTER,
        MODE_CDCHECK,
        MODE_UNKNOWN13,
        MODE_SNOWBOARD2,
        MODE_UNKNOWN15,
        MODE_UNKNOWN16,
        MODE_BATTLE_MENU,
        MODE_UNKNOWN18,
        MODE_EXIT,
        MODE_MAIN_MENU,
        MODE_UNKNOWN21,
        MODE_UNKNOWN22,
        MODE_SWIRL,
        MODE_UNKNOWN24,
        MODE_UNKNOWN25,
        MODE_GAMEOVER,
        MODE_CREDITS,
        MODE_UNKNOWN28,
    };

    typedef struct {
        GameModes mode;
        char* name;
        bool trace;
        uint32_t mainLoop;
        uint32_t frameRate;
    } GameMode;

    class GameModeDetect {
    private:
        GameContext* gameContext;

        GameMode modes[FF7DRIVER_NUM_MODES]{
            {MODE_FIELD,       "MODE_FIELD",       false},
            {MODE_BATTLE,      "MODE_BATTLE",      false},
            {MODE_WORLDMAP,    "MODE_WORLDMAP",    false},
            {MODE_UNKNOWN4,    "MODE_UNKNOWN4",    true },
            {MODE_MENU,        "MODE_MENU",        false},
            {MODE_HIGHWAY,     "MODE_HIGHWAY",     false},
            {MODE_CHOCOBO,     "MODE_CHOCOBO",     false},
            {MODE_SNOWBOARD,   "MODE_SNOWBOARD",   false},
            {MODE_CONDOR,      "MODE_CONDOR",      false},
            {MODE_SUBMARINE,   "MODE_SUBMARINE",   false},
            {MODE_COASTER,     "MODE_COASTER",     false},
            {MODE_CDCHECK,     "MODE_CDCHECK",     false},
            {MODE_UNKNOWN13,   "MODE_UNKNOWN13",   true },
            {MODE_SNOWBOARD2,  "MODE_SNOWBOARD2",  false},
            {MODE_UNKNOWN15,   "MODE_UNKNOWN15",   true },
            {MODE_UNKNOWN16,   "MODE_UNKNOWN16",   true },
            {MODE_BATTLE_MENU, "MODE_BATTLE_MENU", false},
            {MODE_UNKNOWN18,   "MODE_UNKNOWN18",   true },
            {MODE_EXIT,        "MODE_EXIT",        false},
            {MODE_MAIN_MENU,   "MODE_MAIN_MENU",   false},
            {MODE_UNKNOWN21,   "MODE_UNKNOWN21",   true },
            {MODE_UNKNOWN22,   "MODE_UNKNOWN22",   true },
            {MODE_SWIRL,       "MODE_SWIRL",       false},
            {MODE_UNKNOWN24,   "MODE_UNKNOWN24",   true },
            {MODE_UNKNOWN25,   "MODE_UNKNOWN25",   true },
            {MODE_GAMEOVER,    "MODE_GAMEOVER",    false},
            {MODE_CREDITS,     "MODE_CREDITS",     false},
            {MODE_UNKNOWN28,   "MODE_UNKNOWN28",   true },
        };

        WORD* modeFlag;

        void setMainLoop(uint32_t inMode, uint32_t main_loop)
        {
            uint32_t i;

            for (i = 0; i < FF7DRIVER_NUM_MODES; i++)
                if (modes[i].mode == inMode)
                    modes[i].mainLoop = main_loop;
        }

        void setModeFramerate(uint32_t inMode, uint32_t framerate)
        {
            uint32_t i;

            for (i = 0; i < FF7DRIVER_NUM_MODES; i++)
                if (modes[i].mode == inMode)
                    modes[i].frameRate = framerate;
        }

    public:
        GameModeDetect() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        void init(GameContext* inContext) {
            gameContext = inContext;

            // Init game modes
            modeFlag = (WORD*)FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_FLAG_MODE);
            setMainLoop(MODE_GAMEOVER, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_GAMEOVER));
            setMainLoop(MODE_SWIRL, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_SWIRL));
            setMainLoop(MODE_CDCHECK, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_CDCHECK));
            setMainLoop(MODE_CREDITS, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_CREDITS));
            setMainLoop(MODE_MENU, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_MENU));
            setMainLoop(MODE_BATTLE, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_BATTLE));
            setMainLoop(MODE_FIELD, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_FIELD));
            setMainLoop(MODE_WORLDMAP, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_WORLDMAP));
            setMainLoop(MODE_CHOCOBO, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_CHOCOBO));
            setMainLoop(MODE_CONDOR, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_CONDOR));
            setMainLoop(MODE_HIGHWAY, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_HIGHWAY));
            setMainLoop(MODE_COASTER, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_COASTER));
            setMainLoop(MODE_SUBMARINE, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_SUBMARINE));
            setMainLoop(MODE_SNOWBOARD, FFNx::Offset::getAbsoluteValue(FF7_OFFSET_MAIN_LOOP, FF7_MODE_SNOWBOARD));
        }

        GameMode* getMode() {
            uint32_t last_mode = 0;
            uint32_t i;

            // find exact match, mode and main loop both match
            for (i = 0; i < FF7DRIVER_NUM_MODES; i++)
            {
                GameMode* m = &modes[i];

                if (m->mainLoop == (uint32_t)gameContext->mainCallbacksA0C.main && m->mode == *modeFlag)
                {
                    if (last_mode != m->mode)
                    {
                        last_mode = m->mode;
                    }

                    return m;
                }
            }

            // if there is no exact match, try to find a match by main loop only
            for (i = 0; i < FF7DRIVER_NUM_MODES; i++)
            {
                GameMode* m = &modes[i];

                if (m->mainLoop && m->mainLoop == (uint32_t)gameContext->mainCallbacksA0C.main)
                {
                    if (last_mode != m->mode)
                    {
#ifndef RELEASE
                        if (m->mode != *modeFlag && m->trace)
                        {
                            uint32_t j;
                            GameMode* _m;

                            for (j = 0; j < FF7DRIVER_NUM_MODES - 1; j++)
                            {
                                _m = &modes[j];

                                if (_m->mode == *modeFlag) break;
                            }
                        }
#endif
                        last_mode = m->mode;
                    }

                    return m;
                }
            }

            // finally, ignore main loop and try to match by mode only
            for (i = 0; i < FF7DRIVER_NUM_MODES; i++)
            {
                GameMode* m = &modes[i];

                if (m->mode == *modeFlag)
                {
                    if (last_mode != m->mode)
                    {
                        last_mode = m->mode;
                    }

                    return m;
                }
            }

            if (*modeFlag != last_mode)
            {
                ffDriverLog.write("unknown mode (%i, 0x%x)\n", *modeFlag, (uint32_t)gameContext->mainCallbacksA0C.main);
                last_mode = *modeFlag;
            }

            return &modes[4];
        }

    };

}

#endif // !__FF7DRIVER_MODE_HPP__
