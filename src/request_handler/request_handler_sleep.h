#ifndef REQUEST_HANDLER_SLEEP_H
#define REQUEST_HANDLER_SLEEP_H

#include "../config_parser.h"
#include "request_handler.h"
#include <boost/beast/http.hpp>

class RequestHandlerSleep : public RequestHandler {
public:
  explicit RequestHandlerSleep();
  std::string getName() noexcept override;
  void handleRequest(const Request &request_,
                     Response *response_) noexcept override;
};

#endif // REQUEST_HANDLER_SLEEP_H


