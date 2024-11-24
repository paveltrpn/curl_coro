
#pragma once

#include <coroutine>

#include "web_client.h"

template <typename T>
struct Task;

template <>
struct Task<void> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        Task get_return_object() { return handle_type::from_promise( *this ); };
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    Task( handle_type h )
        : handle_( h ) {}

private:
    handle_type handle_;
};

template <>
struct Task<Result> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        Task get_return_object() { return handle_type::from_promise( *this ); };
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    Task( handle_type h )
        : handle_( h ) {}

    Result get() { return {}; }

private:
    handle_type handle_;
    Result payload_;
};

struct RequestAwaitable {
    RequestAwaitable( Poller& client_, std::string url_ )
        : client( client_ )
        , url( std::move( url_ ) ){};

    bool await_ready() const noexcept { return false; }
    void await_suspend(
        std::coroutine_handle<Task<void>::promise_type> handle ) noexcept {
        client.performRequest( std::move( url ), [handle, this]( Result res ) {
            result = std::move( res );
            handle.resume();
        } );
    }
    Result await_resume() const noexcept { return std::move( result ); }

    Poller& client;
    std::string url;
    Result result;
};
