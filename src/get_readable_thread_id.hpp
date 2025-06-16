#pragma once
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

#include <thread>
#include <map>
#include <mutex>

uint32_t get_readable_thread_id()
{
    static std::mutex mtx;
    static std::map<std::thread::id, uint32_t> ids;
    static uint32_t next_id = 1;

    std::lock_guard<std::mutex> lock(mtx);
    auto current_id = std::this_thread::get_id();
    auto it = ids.find(current_id);

    if (it == ids.end()) {
        ids[current_id] = next_id++;
        return ids[current_id];
    }
    else
        return it->second;
}
