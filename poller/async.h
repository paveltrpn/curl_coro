
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

    Task( Task&& t ) noexcept
        : handle_( t.handle_ ) {
        t.handle_ = nullptr;
    }

    Task& operator=( Task&& other ) noexcept {
        if ( std::addressof( other ) != this ) {
            if ( handle_ ) {
                handle_.destroy();
            }

            handle_ = other.handle_;
            other.handle_ = nullptr;
        }

        return *this;
    }

    Task( const Task& ) = delete;
    Task& operator=( const Task& ) = delete;

    // ~Task() {
    // if ( handle_ ) handle_.destroy();
    // }

private:
    handle_type handle_;
};

namespace __detail__ {
static std::condition_variable cv_;
static std::mutex mtx_;
}  // namespace __detail__

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
            __detail__::cv_.notify_all();
            return {};
        }

        void unhandled_exception() { exception_ = std::current_exception(); }
    };

    Task( handle_type h )
        : handle_( h ) {}

    Task( Task&& t ) noexcept
        : handle_( t.handle_ ) {
        t.handle_ = nullptr;
    }

    Task& operator=( Task&& other ) noexcept {
        if ( std::addressof( other ) != this ) {
            if ( handle_ ) {
                handle_.destroy();
            }

            handle_ = other.handle_;
            other.handle_ = nullptr;
        }

        return *this;
    }

    Task( const Task& ) = delete;
    Task& operator=( const Task& ) = delete;

    ~Task() {
        if ( handle_ ) handle_.destroy();
    }

    Result get() {
        std::unique_lock<std::mutex> lk{ __detail__::mtx_ };
        __detail__::cv_.wait( lk, [this]() { return handle_.done(); } );

        return handle_.promise().payload_;
    }

private:
    handle_type handle_;
};

struct RequestAwaitable {
    RequestAwaitable( Poller& client )
        : client_( client ){};

    // HTTP request always NOT ready immedieateley!
    bool await_ready() const noexcept { return false; }

    Result await_resume() const noexcept { return std::move( result_ ); }

    Poller& client_;
    Result result_;
};

template <typename T>
struct StringRequestAwaitable final : RequestAwaitable {
    StringRequestAwaitable( Poller& client, std::string url )
        : RequestAwaitable( client )
        , url( std::move( url ) ){};

    void await_suspend(
        std::coroutine_handle<typename T::promise_type> handle ) noexcept {
        client_.performRequest( std::move( url ), [handle, this]( Result res ) {
            result_ = std::move( res );
            handle.resume();
        } );
    }

    std::string url;
};

template <typename T>
struct HttpRequestAwaitable final : RequestAwaitable {
    HttpRequestAwaitable( Poller& client, HttpRequest request )
        : RequestAwaitable( client )
        , request_( std::move( request ) ){};

    void await_suspend(
        std::coroutine_handle<typename T::promise_type> handle ) noexcept {
        client_.performRequest( std::move( request_ ),
                                [handle, this]( Result res ) {
                                    result_ = std::move( res );
                                    handle.resume();
                                } );
    }

    HttpRequest request_;
};

}  // namespace poller
