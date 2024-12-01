
#include "request.h"

namespace poller {

HttpRequest::HttpRequest( const std::string& url,
                          const std::string& userAgent ) {
    handle_.setopt<CURLOPT_URL>( url );
    handle_.setopt<CURLOPT_USERAGENT>( userAgent );
};

HttpRequest::HttpRequest( HttpRequest&& other ) {
    handle_ = std::move( other.handle_ );
}

HttpRequest& HttpRequest::operator=( HttpRequest&& other ) {
    if ( this != &other ) {
        handle_ = std::move( other.handle_ );
    }
    return *this;
}

}  // namespace poller
