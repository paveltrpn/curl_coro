
#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <curl/curl.h>

#include "request.h"

namespace poller {

struct Result {
    int code;
    std::string data;
};

using CallbackFn = std::function<void( Result result )>;

struct Request {
    CallbackFn callback;
    std::string buffer;
};

template <typename T>
struct Task;

template <typename T>
struct StringRequestAwaitable;

template <typename T>
struct HttpRequestAwaitable;

class Poller {
public:
    Poller();
    ~Poller();

    Poller( const Poller& other ) = delete;
    Poller( Poller&& other ) = delete;
    Poller& operator=( const Poller& other ) = delete;
    Poller& operator=( Poller&& other ) = delete;

    void stop();

    void performRequest( const HttpRequest& request, CallbackFn cb ) = delete;
    HttpRequestAwaitable<Task<void>> requestAsync(
        const HttpRequest& request ) = delete;

    void performRequest( const std::string& url, CallbackFn cb );
    void performRequest( HttpRequest&& request, CallbackFn cb );

    StringRequestAwaitable<Task<void>> requestAsyncVoid( std::string url );
    StringRequestAwaitable<Task<Result>> requestAsyncPromise( std::string url );
    HttpRequestAwaitable<Task<void>> requestAsyncVoid( HttpRequest&& request );
    HttpRequestAwaitable<Task<Result>> requestAsyncPromise(
        HttpRequest&& request );

private:
    void run();

private:
    std::unique_ptr<std::thread> worker_;

    CURLM* multiHandle_;
    std::atomic_bool break_{ false };
};

}  // namespace poller
