#ifndef __FFNX_CRASHHANDLER_HPP__
#define __FFNX_CRASHHANDLER_HPP__

#include "common.h"

namespace FFNx {

#define FFNX_STACK_MAX_NAME_LENGTH 256
#define FFNX_DUMP_FILENAME _DLLOUTNAME ".dmp"

    class CrashHandler {
    public:
        // Prints stack trace based on context record
        // Via https://stackoverflow.com/a/50208684
        void printStack(CONTEXT* ctx)
        {
            BOOL    result;
            HANDLE  process;
            HANDLE  thread;
            HMODULE hModule;

            STACKFRAME        stack;
            ULONG             frame;
            DWORD64           displacement;

            DWORD disp;
            IMAGEHLP_LINE* line;

            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            char name[FFNX_STACK_MAX_NAME_LENGTH];
            char module[FFNX_STACK_MAX_NAME_LENGTH];
            PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

            memset(&stack, 0, sizeof(STACKFRAME));

            process = GetCurrentProcess();
            thread = GetCurrentThread();
            displacement = 0;
            stack.AddrPC.Offset = (*ctx).Eip;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrStack.Offset = (*ctx).Esp;
            stack.AddrStack.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = (*ctx).Ebp;
            stack.AddrFrame.Mode = AddrModeFlat;

            SymInitialize(process, NULL, TRUE); //load symbols

            for (frame = 0; ; frame++)
            {
                //get next call from stack
                result = StackWalk
                (
                    IMAGE_FILE_MACHINE_I386,
                    process,
                    thread,
                    &stack,
                    ctx,
                    NULL,
                    SymFunctionTableAccess,
                    SymGetModuleBase,
                    NULL
                );

                if (!result) break;

                //get symbol name for address
                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = MAX_SYM_NAME;
                SymFromAddr(process, (ULONG64)stack.AddrPC.Offset, &displacement, pSymbol);

                line = (IMAGEHLP_LINE*)malloc(sizeof(IMAGEHLP_LINE));
                line->SizeOfStruct = sizeof(IMAGEHLP_LINE);

                //try to get line
                if (SymGetLineFromAddr(process, stack.AddrPC.Offset, &disp, line))
                {
                    ffDriverLog.write("\tat %s in %s: line: %lu: address: 0x%I64x\n", pSymbol->Name, line->FileName, line->LineNumber, pSymbol->Address);
                }
                else
                {
                    hModule = NULL;
                    lstrcpyA(module, "");
                    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCTSTR)(stack.AddrPC.Offset), &hModule);

                    //at least print module name
                    if (hModule != NULL) GetModuleFileNameA(hModule, module, FFNX_STACK_MAX_NAME_LENGTH);

                    ffDriverLog.write("in %s\n", module);

                    //failed to get line
                    ffDriverLog.write("\tat %s, address 0x%I64x\n", pSymbol->Name, pSymbol->Address);
                }

                free(line);
                line = NULL;
            }
        }
    };

    LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ep)
    {
        static bool had_exception = false;
        FFNx::CrashHandler handler;

        // give up if we crash again inside the exception handler (this function)
        if (had_exception)
        {
            ffDriverLog.error("ExceptionHandler", "Crash while running another ExceptionHandler. Exiting.");
            SetUnhandledExceptionFilter(0);
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        ffDriverLog.write("*** Exception 0x%x, address 0x%x ***", ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);
        handler.printStack(ep->ContextRecord);

        had_exception = true;

        // show cursor in case it was hidden
        ShowCursor(true);

        // save crash dump to game directory
        HANDLE file = CreateFile(FFNX_DUMP_FILENAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        HANDLE proc = GetCurrentProcess();
        DWORD procid = GetCurrentProcessId();
        MINIDUMP_EXCEPTION_INFORMATION mdei;

        CONTEXT c;
        HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());;
        memset(&c, 0, sizeof(c));
        c.ContextFlags = CONTEXT_FULL;
        GetThreadContext(hThread, &c);

        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = ep;
        mdei.ExceptionPointers->ContextRecord = &c;
        mdei.ClientPointers = false;

        if (!MiniDumpWriteDump(proc, procid, file, MiniDumpNormal, &mdei, NULL, NULL)) {
            wchar_t buf[256];

            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                buf, (sizeof(buf) / sizeof(wchar_t)), NULL);

            ffDriverLog.write("MiniDumpWriteDump failed with error: %ls\n", buf);
        }


        ffDriverLog.error("Unhandled Exception", "An error occoured and a stacktrace have been dumped. See the log file for more details.\n");

        // Execute default exception handler next
        return EXCEPTION_EXECUTE_HANDLER;;
    }

}

#endif
