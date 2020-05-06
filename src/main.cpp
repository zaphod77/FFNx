#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif

__declspec(dllexport) void* new_dll_graphics_driver(GameContext* gameContext)
{
    FFNx::DetectGame detectGame;

    // Obtain Game window informations
    ffWindow.setHwnd(gameContext->hwnd);
    ffWindow.setClassNamePtr(gameContext->wndClassNamePtr);
    ffWindow.setHandleInstance(gameContext->hinstance);

    // Install crash handler
    SetUnhandledExceptionFilter(FFNx::ExceptionHandler);

    // Prevent Screensavers
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

    if (detectGame.isFF7()) {
        ffDriverLog.info("Game Detected", "Detected Final Fantasy VII. Enjoy!");

        // Required to allow our driver to work properly
        gameContext->nvidiaFix = 0;
        gameContext->d3d2_flag = 1;

        return ff7Driver.init(gameContext);
    } else if(detectGame.isFF8()) {
        // Run FF8 Driver
        ffDriverLog.warning("Game Detected", "Final Fantasy VIII is not supported yet by this driver.");
        exit(1);
    } else {
        // Unsupported
        ffDriverLog.error("Not supported", "This driver does not support the version of your game. Please make sure to use only US editions of Final Fantasy VII and Final Fantasy VIII.");
    }

    return NULL;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        ffDriverLog.write("Game is loading " _DLLOUTNAME " DLL.");
    else if (fdwReason == DLL_PROCESS_DETACH)
        ffDriverLog.write("Game is unloading " _DLLOUTNAME " DLL.");

    return TRUE;
}

#if defined(__cplusplus)
}
#endif
