#ifndef __FFNX_WINDOW_HPP__
#define __FFNX_WINDOW_HPP__

#include "common.h"

namespace FFNx {

    enum WindowMeta {
        TITLE,
        BACKEND,
        _NumItems
    };

    class Window {
    private:
        HWND hWnd;
        HINSTANCE hInstance;
        char* classNamePtr;

        // Widow state
        bool isWindowFullscreen;

        // Current user resolution
        DEVMODE currentUserScreen;

        // Current window title
        std::string metaInformation[WindowMeta::_NumItems];

        // Screen resolution
        uint32_t windowWidth = 0;
        uint32_t windowHeight = 0;

        void setWindowStyle() {
            RECT windowRect;
            uint32_t initialWindowsPositionOffsetX = (currentUserScreen.dmPelsWidth / 2) - (windowWidth / 2);
            uint32_t initialWindowsPositionOffsetY = (currentUserScreen.dmPelsHeight / 2) - (windowHeight / 2);

            windowRect.top = 0;
            windowRect.right = windowWidth;
            windowRect.bottom = windowHeight;
            windowRect.left = 0;

            DestroyWindow(hWnd);

            if (!AdjustWindowRectEx(&windowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, false, 0))
                ffDriverLog.error("Error creating window", "Could not create the game Window. Refer to the log for more details.");

            if (!(hWnd = CreateWindowEx(0, classNamePtr, "", WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, initialWindowsPositionOffsetX, initialWindowsPositionOffsetY, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), 0, 0, hInstance, 0)))
                ffDriverLog.error("Error creating window", "Could not create the game Window. Refer to the log for more details.");

            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(101)));

            ShowWindow(hWnd, SW_SHOW);
        }

        void setResolution() {
            windowWidth = ffUserConfig.getWidth();
            windowHeight = ffUserConfig.getHeight();

            if (windowWidth == 0 || windowHeight == 0) {
                windowWidth = currentUserScreen.dmPelsWidth;
                windowHeight = currentUserScreen.dmPelsHeight;
            }
        }

    public:
        Window() {
            ffDriverLog.write("Initializing %s", __func__);
        }

        void init() {
            // Read current user resolution
            EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &currentUserScreen);

            setResolution();
            setWindowStyle();
            resetWindowTitle();
        }

        uint32_t getWidth() {
            return windowWidth;
        }

        uint32_t getHeight() {
            return windowHeight;
        }

        HWND getHWnd() {
            return hWnd;
        }

        void setHwnd(HWND newhWnd) {
            hWnd = newhWnd;
        }

        void setClassNamePtr(char* newClassName) {
            classNamePtr = newClassName;
        }

        void setHandleInstance(HINSTANCE newHInstance) {
            hInstance = newHInstance;
        }

        void setMetaInfo(WindowMeta type, std::string info) {
            metaInformation[type] = info;
        }

        void resetWindowTitle() {
            std::string windowTitle;

            for (int i = 0; i < _NumItems; i++) {
                if (i > 0) windowTitle += " | ";
                windowTitle += metaInformation[i];
            }

            SetWindowText(hWnd, windowTitle.c_str());
        }
    };

}

#endif
