#include "request_handler_echo.h"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace http = boost::beast::http;

std::string RequestHandlerEcho::getName() noexcept {
    return "EchoHandler";
}
/**
 * Constructor - Initialize the echo handler.
 */
RequestHandlerEcho::RequestHandlerEcho() {
  // Optionally, process the config here
  // std::cout << "RequestHandlerEcho initialized with config." << std::endl;
}

/**
 * handleRequest() - Return reply same as request.
 */
void RequestHandlerEcho::handleRequest(const Request &request_,
                                       Response *response_) noexcept {
  std::cout << "RequestHandlerEcho::handleRequest()" << std::endl;
  response_->version(request_.version());
  response_->result(http::status::ok);
  // Echo back the request as the response body
  response_->body() = requestToString(request_);

  // Set headers after the body to ensure content length is calculated correctly
  response_->set(http::field::content_type, "text/plain");
  response_->content_length(response_->body().size());

  // Prepare the payload to ensure headers and body are correctly formatted
  response_->prepare_payload();
}

/**
 * requestToString() - Convert boost::beast::http::request struct to
 * std::string.
 */
std::string RequestHandlerEcho::requestToString(const Request &req) {
    std::ostringstream oss;

    // Start line
    oss << req.method_string() << " " << req.target() << " HTTP/" << (req.version() / 10) << "." << (req.version() % 10) << "\r\n";

    // Headers
    for (const auto& field : req.base()) {
        oss << field.name_string() << ": " << field.value() << "\r\n";
    }

    // End of headers
    oss << "\r\n";

    // Body
    oss << req.body();

    return oss.str();
}