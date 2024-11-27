
#pragma once

#include <string>
#include <concepts>
#include <curl/curl.h>

namespace poller {

template <CURLoption Opt>
concept CurlOptCallable = ( Opt == CURLOPT_WRITEFUNCTION );

template <CURLoption Opt>
concept CurlOptObject = ( Opt == CURLOPT_WRITEDATA ) ||
                        ( Opt == CURLOPT_PRIVATE );

template <CURLoption Opt>
concept CurlOptString = ( Opt == CURLOPT_URL ) ||
                        ( Opt == CURLOPT_USERAGENT ) ||
                        ( Opt == CURLOPT_POSTFIELDS );

template <CURLoption Opt>
concept CurlOptLong = ( Opt == CURLOPT_HEADER ) || ( Opt == CURLOPT_POST );

struct Handle final {
    Handle();
    ~Handle() = default;

    Handle( const Handle& other ) = delete;
    Handle( Handle&& other ) = delete;
    Handle& operator=( const Handle& other ) = delete;
    Handle& operator=( Handle&& other ) = delete;

    // TODO: T - bad option, must be invokable
    template <CURLoption Opt, typename T>
    requires CurlOptCallable<Opt> void setopt( T value ) {
        curl_easy_setopt( handle_, Opt, value );
    };

    template <CURLoption Opt>
    requires CurlOptString<Opt> void setopt( const std::string& value ) {
        curl_easy_setopt( handle_, Opt, value.c_str() );
    };

    template <CURLoption Opt>
    requires CurlOptLong<Opt> void setopt( long value ) {
        curl_easy_setopt( handle_, Opt, value );
    };

    template <CURLoption Opt, typename T>
    requires CurlOptObject<Opt> void setopt( T value ) {
        curl_easy_setopt( handle_, Opt, value );
    };

    void setopt( CURLoption opt, void* value ) {
        curl_easy_setopt( handle_, opt, value );
    };

    CURL* get() { return handle_; };

private:
    CURL* handle_;
};

}  // namespace poller
