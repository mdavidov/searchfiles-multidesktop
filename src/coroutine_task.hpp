#pragma once

#include <coroutine>
#include <exception>
#include <future>
#include <cstdint>
#include <iostream>
#include <thread>


template<typename T>
struct Task {
    struct promise_type {
        T result;
        std::exception_ptr exception = nullptr;
        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(T value) { result = value; }
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) {
        // Example enhancement
        std::thread([h]() {
            // Do some async work
            h.resume();  // Resume the coroutine
        })
        .detach();
    }
    T await_resume() {
        return coro.promise().result;
    }

    Task(std::coroutine_handle<promise_type> ch) : coro(ch) {
    }
    ~Task() {
        if (coro) coro.destroy();
    }

    T get_result() {
        if (coro.promise().exception) {
            std::rethrow_exception(coro.promise().exception);
        }
        return coro.promise().result;
    }

    std::coroutine_handle<promise_type> coro;
};


template<>
struct Task<void> {
    struct promise_type {
        std::exception_ptr exception = nullptr;
        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    Task(std::coroutine_handle<promise_type> ch) : coro(ch) {
    }
    ~Task() {
        if (coro) coro.destroy();
    }

    void get_result() {
        if (coro.promise().exception) {
            std::rethrow_exception(coro.promise().exception);
        }
    }

    std::coroutine_handle<promise_type> coro;
};
