#ifndef __FFNX_UTILS_HPP__
#define __FFNX_UTILS_HPP__

#include "common.h"

// Utility functions
namespace FFNx {
    namespace Utils {
        char* getGameBaseDirectory() {
            char baseDir[512];

            GetModuleFileName(NULL, baseDir, sizeof(baseDir));
            PathRemoveFileSpec(baseDir);

            return baseDir;
        }

        // Via https://stackoverflow.com/a/14375308
        uint32_t createBGRA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            return ((b & 0xff) << 24) + ((g & 0xff) << 16) + ((r & 0xff) << 8) + (a & 0xff);
        }
    }

    namespace Offset {
        uint32_t getRelativeCall(uint32_t base, uint32_t offset)
        {
            return base + *((uint32_t*)(base + offset + 1)) + offset + 5;
        }

        uint32_t getAbsoluteValue(uint32_t base, uint32_t offset)
        {
            return *((uint32_t*)(base + offset));
        }
    }

    namespace Memory {
        void memcpyCode(uint32_t offset, void* data, uint32_t size)
        {
            DWORD dummy;

            VirtualProtect((void*)offset, size, PAGE_EXECUTE_READWRITE, &dummy);

            memcpy((void*)offset, data, size);
        }

        void memsetCode(uint32_t offset, uint32_t val, uint32_t size)
        {
            DWORD dummy;

            VirtualProtect((void*)offset, size, PAGE_EXECUTE_READWRITE, &dummy);

            memset((void*)offset, val, size);
        }

        class Patch {
        private:
            uint32_t replaceCounter = 0;
            uint32_t replacedFunctions[512 * 3];

        public:
            uint32_t replaceFunction(uint32_t offset, void* func)
            {
                DWORD dummy;

                VirtualProtect((void*)offset, 5, PAGE_EXECUTE_READWRITE, &dummy);

                replacedFunctions[replaceCounter++] = *(uint8_t*)offset;
                replacedFunctions[replaceCounter++] = *(uint32_t*)(offset + 1);
                replacedFunctions[replaceCounter++] = offset;

                *(uint8_t*)offset = 0xE9;
                *(uint32_t*)(offset + 1) = ((uint32_t)func - offset) - 5;

                return replaceCounter - 3;
            }

            void unreplaceFunctions()
            {
                while (replaceCounter > 0)
                {
                    uint32_t offset = replacedFunctions[--replaceCounter];
                    DWORD dummy;

                    VirtualProtect((void*)offset, 5, PAGE_EXECUTE_READWRITE, &dummy);
                    *(uint32_t*)(offset + 1) = replacedFunctions[--replaceCounter];
                    *(uint8_t*)offset = replacedFunctions[--replaceCounter];
                }
            }
        };
    }

    class DetectGame {
    private:
        uint32_t version_check[1][2];

        uint32_t get_version(uint32_t offset) {
            return (*(uint32_t*)(offset));
        }

        template <size_t rows, size_t cols>
        bool match_version(uint32_t(&game_version)[rows][cols]) {
            bool ret = false;

            for (int row = 0; row < std::size(game_version); row++) {
                for (int col = 0; col < std::size(game_version[row]); col++) {
                    ret = (version_check[0][col] == game_version[row][col]);
                }
                if (ret) break;
            }

            return ret;
        }

    public:
        DetectGame() {
            version_check[0][0] = get_version(0x401004);
            version_check[0][1] = get_version(0x401404);
        }

        bool isFF7() {
            uint32_t ff7_versions[1][1] = {
                0x99CE0805 // FF7 1.02 US English
            };

            return match_version(ff7_versions);
        }

        bool isFF8() {
            uint32_t ff8_versions[4][2] = {
                0x3885048D, 0x159618, // FF8 1.2 US English
                0x3885048D, 0x1597C8, // FF8 1.2 US English (Nvidia)
                0x2885048D, 0x159598, // FF8 1.2 US English (Eidos Patch)
                0x2885048D, 0x159748, // FF8 1.2 US English (Eidos Patch) (Nvidia)
            };

            return match_version(ff8_versions);
        }
    };
}

#endif
