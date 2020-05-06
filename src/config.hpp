#ifndef __FFNX_CONFIG_HPP__
#define __FFNX_CONFIG_HPP__

#include "common.h"

#include <fstream>
#include <iostream>
#include <iomanip> 
#include <nlohmann/json.hpp>

namespace FFNx {

#define FFNX_CONFIG_FILENAME _DLLOUTNAME ".json"

    class UserConfig {
    private:
        bool must_save_on_exit = false;

        nlohmann::json config;
        nlohmann::json userConfig;


        void load_defaults() {
            // Enabled direct mode support
            config["directMode"]["enabled"] = false;

            // Use window mode by default
            config["fullscreen"] = false;

            // Default resolution to 640x480
            config["resolution"]["width"] = 640;
            config["resolution"]["height"] = 480;

            // Preserve aspect ratio by default
            config["preserve_aspect_ratio"] = true;

            // Render Type
            config["backend"] = "OpenGL";
            config["vsync"] = true;

            // Music configuration
            config["music"]["path"] = "music\\vgmstream";
            config["music"]["ext"] = "ogg";

            // Shaders
            config["shader"]["fragment"] = _DLLOUTNAME ".frag.bin";
            config["shader"]["vertex"] = _DLLOUTNAME ".vert.bin";

        };

        void log_config() {
            ffDriverLog.write("---- Using configuration ----");
            ffDriverLog.write("%s\n", config.dump(4).c_str());
            ffDriverLog.write("-----------------------------");
        }

    public:
        UserConfig() {
            ffDriverLog.write("Initializing %s", __func__);

            load_defaults();

            std::ifstream i(FFNX_CONFIG_FILENAME);

            if (!i.fail()) {
                i >> userConfig;
                config.merge_patch(userConfig);
            }
            else
                must_save_on_exit = true;

            log_config();
        }

        void flush() {
            if (must_save_on_exit) {
                std::ofstream o(FFNX_CONFIG_FILENAME);
                o << std::setw(4) << config << std::endl;
            }
        }

        // User configuration
        bool wantsDirectMode() {
            return config["directMode"]["enabled"];
        }

        bool wantsFullscreen() {
            return config["fullscreen"];
        }

        bool keepAspectRatio() {
            return config["preserve_aspect_ratio"];
        }

        uint32_t getWidth() {
            return config["resolution"]["width"];
        }

        uint32_t getHeight() {
            return config["resolution"]["height"];
        }

        std::string getBackend() {
            return config["backend"];
        }

        bool wantsVSync() {
            return config["vsync"];
        }

        std::string getMusicPath() {
            return config["music"]["path"];
        }

        std::string getMusicExt() {
            return config["music"]["ext"];
        }

        std::string getShaderFragmentPath() {
            return config["shader"]["fragment"];
        }

        std::string getShaderVertexPath() {
            return config["shader"]["vertex"];
        }
    };

}

#endif
