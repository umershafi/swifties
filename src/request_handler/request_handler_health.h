#ifndef REQUEST_HANDLER_HEALTH_H
#define REQUEST_HANDLER_HEALTH_H

#include "../config_parser.h"
#include "request_handler.h"
#include <boost/beast/http.hpp>

class RequestHandlerHealth : public RequestHandler {
public:
  explicit RequestHandlerHealth();
  std::string getName() noexcept override;
  void handleRequest(const Request &request_,
                     Response *response_) noexcept override;
};

#endif // REQUEST_HANDLER_HEALTH_H


