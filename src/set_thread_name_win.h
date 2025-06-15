#pragma once

#include <windows.h>

void set_thread_name_win(const char* threadName)
{
#pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO {
        DWORD dwType;     // Must be 0x1000
        LPCSTR szName;    // Pointer to name (in user addr space)
        DWORD dwThreadID; // Thread ID (-1=caller thread)
        DWORD dwFlags;    // Reserved for future use, must be zero
    } THREADNAME_INFO;
#pragma pack(pop)

#pragma warning(push)
#pragma warning(disable : 6320)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = DWORD(-1); // Current thread
    info.dwFlags = 0;

    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        (void)info;
    }
#pragma warning(pop)

}
