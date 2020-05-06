#ifndef __FFDRIVER_COMMMON_H__
#define __FFDRIVER_COMMMON_H__

#include <iterator>
#include <vector>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#include <dbghelp.h>
#include <Shlwapi.h>
#include <dsound.h>

#include "log.hpp"
FFNx::Log ffDriverLog;

#include "crashHandler.hpp"
FFNx::CrashHandler ffCrashHandler;

#include "config.hpp"
FFNx::UserConfig ffUserConfig;

#include "window.hpp"
FFNx::Window ffWindow;

#include "utils.hpp"
FFNx::Memory::Patch ffPatch;

#include "renderer.hpp"
FFNx::Renderer ffRenderer;

#include "ff7/driver.hpp"
FF7::Driver ff7Driver;

#endif
