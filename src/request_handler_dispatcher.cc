#include "request_handler_dispatcher.h"
#include "api/crud_handler.h"
#include "request_handler/request_handler_404.h"
#include "request_handler/request_handler_api.h"
#include "request_handler/request_handler_echo.h"
#include "request_handler/request_handler_static.h"
#include "request_handler/request_handler_health.h"
#include "request_handler/request_handler_sleep.h"
#include <string>
#include "logger.h"


/**
 * Constructor - Construct the RequestHandler set.
 */
RequestHandlerDispatcher::RequestHandlerDispatcher(const NginxConfig &config) {
  initRequestHandlers(config);
}

/**
 * getRequestHandler() - Return pointer to corresponding request handler object.
 */
std::shared_ptr<RequestHandler>
RequestHandlerDispatcher::getRequestHandler(const std::string &target) const {
  std::string uri = target;

  // Remove trailing slashes
  while (uri.length() > 1 && uri.back() == '/')
    uri.pop_back();

  std::shared_ptr<RequestHandler> matched_handler = nullptr;
  std::string matched_prefix;

  for (const auto &entry : handlers_) {
    if (uri.substr(0, entry.first.length()) == entry.first) {
      if (entry.first.length() > matched_prefix.length()) {
        matched_prefix = entry.first;
        matched_handler = entry.second;
      }
    }
  }

  return matched_handler;
}

/**
 * initRequestHandlers() - Initialize request handlers.
 */
size_t
RequestHandlerDispatcher::initRequestHandlers(const NginxConfig &config) {
  size_t num_registered = 0;
  Logger *logger = Logger::getLogger();
  for (const auto &statement : config.statements_) {
    if (statement->tokens_[0] == "server") {
      if (statement->tokens_.size() != 1)
        continue;

      const auto &child_block = statement->child_block_;
      for (const auto &child_statement : child_block->statements_) {
        if (child_statement->tokens_[0] != "location")
          continue;

        if (registerPath(child_statement->tokens_[1],
                         child_statement->tokens_[2],
                         *child_statement->child_block_)){
          logger->logDebugFile("Initialized handler: " + child_statement->tokens_[2]);
          num_registered++;
        }
      }
      break;
    }
  }
  if (handlers_.find("/") == handlers_.end())
    handlers_["/"] = std::make_shared<RequestHandler404>();
  return num_registered;
}

/**
 * registerPath() - Register a handler path.
 */
bool RequestHandlerDispatcher::registerPath(PathUri path_uri,
                                            const std::string &handler_type,
                                            const NginxConfig &config) {
  while (path_uri.length() > 1 && path_uri.back() == '/')
    path_uri.pop_back();

  if (handlers_.find(path_uri) != handlers_.end())
    return false;

  if (handler_type == "StaticHandler") {
    std::string root = "/";
    for (const auto &statement : config.statements_) {
      if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
        root = statement->tokens_[1];
      }
    }
    handlers_[path_uri] =
        std::make_shared<RequestHandlerStatic>(root, path_uri);
  } else if (handler_type == "EchoHandler")
    handlers_[path_uri] = std::make_shared<RequestHandlerEcho>();
  else if (handler_type == "APIHandler") {
    std::string root = "/";
    for (const auto &statement : config.statements_) {
      if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
        root = statement->tokens_[1];
      }
    }
    CRUDHandler *crud_handler = new CRUDHandler(root);
    handlers_[path_uri] =
        std::make_shared<RequestHandlerAPI>(crud_handler, path_uri);
  } else if (handler_type == "HealthHandler") {
    handlers_[path_uri] = std::make_shared<RequestHandlerHealth>();
  } else if (handler_type == "SleepHandler") {
    handlers_[path_uri] = std::make_shared<RequestHandlerSleep>();
  } else
    return false;

  return true;
}
