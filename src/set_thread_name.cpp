/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Milivoj (Mike) DAVIDOV
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    #include "set_thread_name_win.h"
#elif defined(__linux__)
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
