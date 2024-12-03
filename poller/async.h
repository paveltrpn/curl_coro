
#pragma once

#include <coroutine>
#include <exception>
#include <mutex>
#include <print>

#include "poller.h"
#include "request.h"

namespace poller {

template <typename T>
struct Task;

template <>
struct Task<void> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::exception_ptr exception_{ nullptr };

        Task get_return_object() { return handle_type::from_promise( *this ); };
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}

        void unhandled_exception() { exception_ = std::current_exception(); }
    };

    Task( handle_type h )
        : handle_( h ) {}

    // ~Task() {
    // if ( handle_ ) handle_.destroy();
    // }

private:
    handle_type handle_;
};

static std::condition_variable cv_;
static std::mutex mtx_;

template <>
struct Task<Result> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::exception_ptr exception_{ nullptr };
        Result payload_;

        Task get_return_object() { return handle_type::from_promise( *this ); };

        std::suspend_never initial_suspend() noexcept { return {}; }

        void return_value( Result value ) { payload_ = value; }

        std::suspend_always final_suspend() noexcept {
            cv_.notify_all();
            return {};
        }

        void unhandled_exception() { exception_ = std::current_exception(); }
    };

    Task( handle_type h )
        : handle_( h ) {}

    // ~Task() {
    // if ( handle_ ) handle_.destroy();
    // }

    Result get() {
        std::unique_lock<std::mutex> lk{ mtx_ };
        cv_.wait( lk, [this]() { return handle_.done(); } );

        return handle_.promise().payload_;
    }

private:
    handle_type handle_;
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

struct HttpRequestAwaitable {
    HttpRequestAwaitable( Poller& client, HttpRequest request )
        : client_( client )
        , request_( std::move( request ) ){};

    bool await_ready() const noexcept { return false; }

    void await_suspend(
        std::coroutine_handle<Task<Result>::promise_type> handle ) noexcept {
        client_.performRequest( std::move( request_ ),
                                [handle, this]( Result res ) {
                                    result_ = std::move( res );
                                    handle.resume();
                                } );
    }

    Result await_resume() const noexcept { return std::move( result_ ); }

    Poller& client_;
    HttpRequest request_;
    Result result_;
};

}  // namespace poller
