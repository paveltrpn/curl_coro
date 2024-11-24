
#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <curl/curl.h>

struct Result {
    int code;
    std::string data;
};

using CallbackFn = std::function<void( Result result )>;

struct Request {
    CallbackFn callback;
    std::string buffer;
};

struct RequestAwaitable;

class Poller {
public:
    Poller();
    ~Poller();

    Poller( const Poller& other ) = delete;
    Poller( Poller&& other ) = delete;
    Poller& operator=( const Poller& other ) = delete;
    Poller& operator=( Poller&& other ) = delete;

    void stop();
    void performRequest( const std::string& url, CallbackFn cb );
    RequestAwaitable performRequestAsync( std::string url );

private:
    void run();

private:
    std::unique_ptr<std::thread> worker_;

    CURLM* multiHandle_;
    std::atomic_bool break_{ false };
};
