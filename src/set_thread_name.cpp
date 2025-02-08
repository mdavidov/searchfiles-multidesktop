#ifdef _WIN32
    #include "set_thread_name_win.h"
#elif defined(__linux__)
    #define _GNU_SOURCE
    #include <pthread.h>
#elif defined(__APPLE__)
    #include <pthread.h>
#endif

void set_thread_name(const char* name)
{
    #ifdef _WIN32
        set_thread_name_win(name);
    #elif defined(__linux__)
        pthread_setname_np(pthread_self(), name);
    #elif defined(__APPLE__)
        pthread_setname_np(name);
    #endif
}
