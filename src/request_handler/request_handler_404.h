#ifndef REQUEST_HANDLER_404_H
#define REQUEST_HANDLER_404_H

#include <boost/beast/http.hpp>
#include "request_handler.h"
#include "../config_parser.h"

class RequestHandler404 : public RequestHandler {
public:
    std::string getName() noexcept override;
    explicit RequestHandler404();
    void handleRequest(const Request &request_, Response *response_) noexcept override;
};

#endif // REQUEST_HANDLER_404_H
