
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

class WebClient {
public:
    WebClient();
    ~WebClient();

    void stop();
    void performRequest( const std::string& url, CallbackFn cb );

    RequestAwaitable performRequestAsync( std::string url );

private:
    void run();

    static size_t writeToBuffer( char* ptr, size_t, size_t nmemb, void* tab ) {
        auto r = reinterpret_cast<Request*>( tab );
        r->buffer.append( ptr, nmemb );
        return nmemb;
    }

private:
    std::unique_ptr<std::thread> worker_;

    CURLM* multiHandle_;
    std::atomic_bool break_{ false };
};
