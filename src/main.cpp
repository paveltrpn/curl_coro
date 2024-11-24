
#include <iostream>

#include "web_client.h"
#include "async.h"

Task<void> requestAsync( Poller& client, std::string rqst ) {
    auto resp = co_await client.performRequestAsync( rqst );
    std::cout << rqst << " ready: " << resp.code << " - " << resp.data
              << std::endl;
}

int main( int argc, char** argv ) {
    Poller client;

    std::println( "request postman-echo.com" );
    requestAsync( client, "https://postman-echo.com/get" );

    std::println( "request httpbin.org" );
    requestAsync( client, "http://httpbin.org/user-agent" );

    std::println( "request www.gstatic.com" );
    requestAsync( client, "http://www.gstatic.com/generate_204" );

    requestAsync( client, "https://api.coindesk.com/v1/bpi/currentprice.json" );

    client.performRequest( "http://httpbin.org/ip", []( Result res ) {
        std::cout << "Req2 Code: " << res.code << std::endl;
        std::cout << "Req2 Data: '" << res.data << "'" << std::endl
                  << std::endl;
    } );

    std::println( "request postman-echo.com" );
    client.performRequest( "https://postman-echo.com/get", []( Result res ) {
        std::cout << "Req0 Code: " << res.code << std::endl;
        std::cout << "Req0 Data: '" << res.data << "'" << std::endl
                  << std::endl;
    } );

    std::println( "request www.gstatic.com" );
    client.performRequest(
        "http://www.gstatic.com/generate_204", [&]( Result res1 ) {
            std::cout << "Req1 Code: " << res1.code << std::endl;
            std::cout << "Req1 Data: '" << res1.data << "'" << std::endl
                      << std::endl;
            client.performRequest(
                "http://httpbin.org/user-agent", []( Result res2 ) {
                    std::cout << "Req1-2 Code: " << res2.code << std::endl;
                    std::cout << "Req1-2 Data: '" << res2.data << "'"
                              << std::endl
                              << std::endl;
                } );
        } );

    std::cin.get();

    return 0;
}
