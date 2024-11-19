
#pragma once

#include <coroutine>
#include <iostream>

#include "web_client.h"

template <typename T>
struct Awaitable {
    bool await_ready() const noexcept {
        // а нужно ли нам вообще засыпать, может все уже и так готово
        // и мы можем продолжить сразу?
    }
    void await_suspend( std::coroutine_handle<> handle ) noexcept {
        // здесь мы можем запустить какой-то процесс,
        // по завершению которого нами будет вызван handle.resume()
    }
    T await_resume() const noexcept {
        // здесь мы вернем вызывающей стороне результат операции,
        // ну или void если не хотим ничего возвращать
    }
};

struct TaskVoid {
    struct promise_type {
        TaskVoid get_return_object() { return {}; };
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

struct RequestAwaitable {
    RequestAwaitable( WebClient& client_, std::string url_ )
        : client( client_ )
        , url( std::move( url_ ) ){};

    bool await_ready() const noexcept { return false; }
    void await_suspend( std::coroutine_handle<> handle ) noexcept {
        client.performRequest( std::move( url ), [handle, this]( Result res ) {
            result = std::move( res );
            handle.resume();
        } );
    }
    Result await_resume() const noexcept { return std::move( result ); }

    WebClient& client;
    std::string url;
    Result result;
};

struct CurlResponseTask {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        CurlResponseTask get_return_object() {
            return handle_type::from_promise( *this );
        };
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}

        std::suspend_always await_transform( RequestAwaitable from ) {
            return {};
        }
    };

    CurlResponseTask( handle_type h )
        : handle_( h ) {}

    std::string get() { return {}; }

private:
    handle_type handle_;
    std::string payload_;
};
