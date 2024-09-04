/**
 * Abstract class for request handlers.
 */

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <iostream>
#include <boost/beast/http.hpp>
#include "../config_parser.h"

namespace http = boost::beast::http;

class RequestHandler {
public:
    
    using Request = http::request<http::string_body>;
    using Response = http::response<http::string_body>;

    virtual void handleRequest(const Request &request_, Response *response_) noexcept = 0;
    virtual std::string getName() noexcept = 0;
protected:
    
};

#endif // REQUEST_HANDLER_H
