
#include "web_client.h"
#include "async.h"
#include <print>

WebClient::WebClient() {
    multiHandle_ = curl_multi_init();
    if ( !multiHandle_ ) {
        std::println( "can't create curl multi handle" );
    }

    worker_ = std::make_unique<std::thread>( &WebClient::run, this );
    worker_->detach();
}

WebClient::~WebClient() {
    curl_multi_cleanup( multiHandle_ );
}

void WebClient::performRequest( const std::string& url, CallbackFn cb ) {
    Request* requestPtr = new Request{ std::move( cb ), {} };
    CURL* handle = curl_easy_init();
    curl_easy_setopt( handle, CURLOPT_URL, url.c_str() );
    curl_easy_setopt( handle, CURLOPT_USERAGENT, "curl_coro/0.1" );
    curl_easy_setopt( handle, CURLOPT_WRITEFUNCTION,
                      &WebClient::writeToBuffer );
    curl_easy_setopt( handle, CURLOPT_WRITEDATA, requestPtr );
    curl_easy_setopt( handle, CURLOPT_PRIVATE, requestPtr );

    // curl_easy_setopt( handle, CURLOPT_HEADER, 1 );

    curl_multi_add_handle( multiHandle_, handle );
}

void WebClient::stop() {
    break_ = true;
    curl_multi_wakeup( multiHandle_ );
}

void WebClient::run() {
    int msgs_left;
    int still_running = 1;

    while ( !break_ ) {
        curl_multi_perform( multiHandle_, &still_running );
        curl_multi_poll( multiHandle_, nullptr, 0, 1000, nullptr );

        CURLMsg* msg;
        do {
            msg = curl_multi_info_read( multiHandle_, &msgs_left );
            if ( msg && ( msg->msg == CURLMSG_DONE ) ) {
                CURL* handle = msg->easy_handle;
                CURLcode res;

                int code;
                curl_easy_getinfo( handle, CURLINFO_RESPONSE_CODE, &code );

                Request* requestPtr;
                res =
                    curl_easy_getinfo( handle, CURLINFO_PRIVATE, &requestPtr );

                if ( res )
                    std::println( "error: {}\n", curl_easy_strerror( res ) );

                requestPtr->callback(
                    { code, std::move( requestPtr->buffer ) } );
                curl_multi_remove_handle( multiHandle_, handle );
                curl_easy_cleanup( handle );
                delete requestPtr;
            }
        } while ( !break_ && msg );
    }

    std::println( "curl loop stopped!" );
}

RequestAwaitable WebClient::performRequestAsync( std::string url ) {
    return RequestAwaitable( *this, std::move( url ) );
}
