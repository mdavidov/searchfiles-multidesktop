#if defined(__APPLE__)
    #include <pthread.h>
#elif defined(__linux__)
    #include <pthread.h>
#elif defined(_WIN32)
    #include "set_thread_name_win.h"
#endif

void set_thread_name(const char* name)
{
    #if defined(__APPLE__)
        pthread_setname_np(name);
    #elif defined(__linux__)
        pthread_setname_np(pthread_self(), name);
    #elif defined(_WIN32)
        set_thread_name_win(name);
    #endif
}
