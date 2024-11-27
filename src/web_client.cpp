
#include "web_client.h"
#include "async.h"
#include "handle.h"

#include <print>

std::string curlBuffer;

[[maybe_unused]] static size_t curlWriteFunc( char* data, size_t size,
                                              size_t nmemb,
                                              std::string* buffer )

{
    size_t result{};
    if ( buffer != nullptr ) {
        buffer->append( data, size * nmemb );
        result = size * nmemb;
    }
    return result;
}

[[maybe_unused]] static size_t writeToBuffer( char* ptr, size_t, size_t nmemb,
                                              void* tab ) {
    auto r = reinterpret_cast<Request*>( tab );
    r->buffer.append( ptr, nmemb );
    return nmemb;
}

[[maybe_unused]] static size_t fillRequest( char* ptr, size_t, size_t nmemb,
                                            Request* tab ) {
    tab->buffer.append( ptr, nmemb );
    return nmemb;
}

Poller::Poller() {
    multiHandle_ = curl_multi_init();
    if ( !multiHandle_ ) {
        std::println( "can't create curl multi handle" );
    }

    worker_ = std::make_unique<std::thread>( &Poller::run, this );
    worker_->detach();
}

Poller::~Poller() {
    stop();
    curl_multi_cleanup( multiHandle_ );
}

void Poller::performRequest( const std::string& url, CallbackFn cb ) {
    Request* requestPtr = new Request{ std::move( cb ), {} };
    // CURL* handle = curl_easy_init();
    poller::Handle handle;
    handle.setopt<CURLOPT_URL>( url );
    handle.setopt<CURLOPT_USERAGENT>( "curl_coro/0.1" );
    handle.setopt<CURLOPT_WRITEFUNCTION>( fillRequest );
    handle.setopt<CURLOPT_WRITEDATA>( requestPtr );
    handle.setopt<CURLOPT_PRIVATE>( requestPtr );

    handle.setopt<CURLOPT_HEADER>( 1 );

    // POST parameters
    // curl_easy_setopt(curl, CURLOPT_POST, 1);
    // const char *urlPOST = "login=ИМЯ&password=ПАСС&cmd=login";
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, urlPOST);

    curl_multi_add_handle( multiHandle_, handle.get() );
}

void Poller::stop() {
    break_ = true;
    curl_multi_wakeup( multiHandle_ );
}

void Poller::run() {
    int msgs_left;
    int still_running = 1;

    while ( !break_ ) {
        CURLMcode res{};

        res = curl_multi_perform( multiHandle_, &still_running );

        if ( res != CURLM_OK ) {
            std::println( "curl_multi_perform failed, code {}",
                          curl_multi_strerror( res ) );
            break;
        }

        res = curl_multi_poll( multiHandle_, nullptr, 0, 1000, nullptr );

        if ( res != CURLM_OK ) {
            std::println( "curl_multi_poll failed, code {}",
                          curl_multi_strerror( res ) );
            break;
        }

        CURLMsg* msg;
        do {
            msg = curl_multi_info_read( multiHandle_, &msgs_left );
            if ( msg && ( msg->msg == CURLMSG_DONE ) ) {
                CURL* handle = msg->easy_handle;
                CURLcode res{};

                int code{};
                res =
                    curl_easy_getinfo( handle, CURLINFO_RESPONSE_CODE, &code );
                if ( res != CURLE_OK ) {
                    std::println( "curl_easy_getinfo failed, code {}\n",
                                  curl_easy_strerror( res ) );
                }

                Request* requestPtr{};
                res =
                    curl_easy_getinfo( handle, CURLINFO_PRIVATE, &requestPtr );

                if ( res != CURLE_OK ) {
                    std::println( "curl_easy_getinfo failed, code {}\n",
                                  curl_easy_strerror( res ) );
                }

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

RequestAwaitable Poller::performRequestAsync( std::string url ) {
    return RequestAwaitable( *this, std::move( url ) );
}
