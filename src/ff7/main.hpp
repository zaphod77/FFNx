#ifndef __FF7DRIVER_MAIN_HPP__
#define __FF7DRIVER_MAIN_HPP__

#include "../common.h"

#include <SisterRay/Sisterray.h>

#define FF7DRIVER_BIT_CALC(x) (1 << x)
#define FF7DRIVER_BGRA_B(x) (x & 0xFF)
#define FF7DRIVER_BGRA_G(x) (x >> 8 & 0xFF)
#define FF7DRIVER_BGRA_R(x) (x >> 16 & 0xFF)
#define FF7DRIVER_BGRA_A(x) (x >> 24 & 0xFF)

// dummy TEX version for framebuffer textures
#define FF7DRIVER_FFNX_TEX_VERSION 0xff7

#include "offsets.hpp"

#include "mode.hpp"
FF7::GameModeDetect ff7DetectGameMode;

#include "file.hpp"
FF7::FileManager ff7FileManager;

#include "engine.hpp"
FF7::Engine ff7DriverEngine;

#include "music.hpp"
FF7::Music ff7DriverMusic;

#include "movie.hpp"
FF7::Movie ff7DriverMovie;

#include "callbacks.hpp"

#endif
