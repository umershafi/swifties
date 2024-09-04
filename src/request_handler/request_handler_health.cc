#include "request_handler_health.h"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <iostream>


namespace http = boost::beast::http;

std::string RequestHandlerHealth::getName() noexcept {
    return "HealthHandler";
}
/**
 * Constructor - Initialize the echo handler.
 */
RequestHandlerHealth::RequestHandlerHealth() {
  // Optionally, process the config here
  // std::cout << "RequestHandlerHealth initialized with config." << std::endl;
}

/**
 * handleRequest() - Return reply same as request.
 */
void RequestHandlerHealth::handleRequest(const Request &request_,
                                       Response *response_) noexcept {
  std::cout << "RequestHandlerHealth::handleRequest()" << std::endl;
  response_->version(request_.version());
  response_->result(http::status::ok);
  response_->body() = "OK";

  // Set headers after the body to ensure content length is calculated correctly
  response_->set(http::field::content_type, "text/plain");
  response_->content_length(response_->body().size());

  // Prepare the payload to ensure headers and body are correctly formatted
  response_->prepare_payload();
}
