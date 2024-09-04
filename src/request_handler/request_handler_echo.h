#ifndef REQUEST_HANDLER_ECHO_H
#define REQUEST_HANDLER_ECHO_H

#include <boost/beast/http.hpp>
#include "request_handler.h"
#include "../config_parser.h"

class RequestHandlerEcho : public RequestHandler {
public:
    explicit RequestHandlerEcho();
    std::string getName() noexcept override;
    void handleRequest(const Request &request_, Response *response_) noexcept override;

private:
    std::string requestToString(const Request &request_);
};

#endif // REQUEST_HANDLER_ECHO_H
