/**
 * A factory-pattern style request handler dispatcher.
 */

#ifndef REQUEST_HANDLER_DISPATCHER_H
#define REQUEST_HANDLER_DISPATCHER_H

#include "config_parser.h"
#include "request_handler/request_handler.h"
#include <boost/beast/http.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

typedef std::string PathUri;

class RequestHandlerDispatcher {
public:
  explicit RequestHandlerDispatcher(const NginxConfig &config);

  virtual std::shared_ptr<RequestHandler>
  getRequestHandler(const std::string &target) const;
  bool registerPath(PathUri path_uri, const std::string &handler_type,
                    const NginxConfig &config);
  size_t initRequestHandlers(const NginxConfig &config);

private:
  std::map<PathUri, std::shared_ptr<RequestHandler>> handlers_;
};

#endif // REQUEST_HANDLER_DISPATCHER_H
