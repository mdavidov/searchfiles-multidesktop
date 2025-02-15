#pragma once

#include <coroutine>
#include <future>
#include <iostream>

// A simple task type that works with coroutines
template<typename T>
struct Task {
    struct promise_type {
        T result;
        Task get_return_object() { return Task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(T value) { result = value; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    T get_result() { return handle.promise().result; }
};

// This is similar to async in C#
Task<int> AsyncOperation() {
    // simulate some async work
    std::this_thread::sleep_for(std::chrono::seconds(1));
    co_return 42;  // similar to return in async method
}

// This is similar to await in C#
Task<int> UseAsyncOperation() {
    int result = co_await AsyncOperation();  // similar to await in C#
    co_return result + 1;
}
