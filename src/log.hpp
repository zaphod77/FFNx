#ifndef __FFNX_LOG_HPP__
#define __FFNX_LOG_HPP__

#include "common.h"

namespace FFNx {

#define FFNX_LOG_FILENAME _DLLOUTNAME ".log"

    class Log {
    private:
        FILE* hFile;

    public:
        Log() {
            hFile = fopen(FFNX_LOG_FILENAME, "wb");

            write(_DLLOUTNAME " " _DLLVERSION);

            write("Initializing %s", __func__);
        }

        ~Log() {
            write("Calling %s", __func__);

            fclose(hFile);
        }

        void error(const char* title, const char* text) {
            write("ERROR: [%s] %s", title, text);
        }

        void warning(const char* title, const char* text) {
            write("WARNING: [%s] %s", title, text);
        }

        void info(const char* title, const char* text) {
            write("INFO: [%s] %s", title, text);
        }

        void write(const char* format, ...) {
            char buffer[4096];
            va_list ap;

            va_start(ap, format);
            vsnprintf(buffer, sizeof(buffer), format, ap);
            fwrite(buffer, strlen(buffer), 1, hFile);
            if (buffer[strlen(buffer) - 1] != '\n') fwrite("\n", 1, 1, hFile);
            fflush(hFile);
            va_end(ap);
        }
    };

}

#endif
