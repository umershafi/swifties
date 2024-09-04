// request_handler_static.h
#ifndef REQUEST_HANDLER_STATIC_H
#define REQUEST_HANDLER_STATIC_H

#include <string>
#include <boost/beast/http.hpp>
#include "request_handler.h"
#include "../config_parser.h"
#include "../http/mime_types.h"


using PathUri = std::string;

class RequestHandlerStatic : public RequestHandler {
public:
    RequestHandlerStatic(const std::string rootString, const PathUri &prefix_);

    void handleRequest(const Request &request_, Response *response_) noexcept override;
    std::string getName() noexcept override;
private:
    PathUri prefix;
    std::string root;
};

#endif // REQUEST_HANDLER_STATIC_H
