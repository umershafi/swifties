#include "request_handler_404.h"
#include <boost/beast/http.hpp>
#include <iostream>


namespace http = boost::beast::http;

std::string RequestHandler404::getName() noexcept {
    return "404Handler";
}
/**
 * Constructor - Initialize the echo handler.
 */
RequestHandler404::RequestHandler404() {
  // Optionally, process the config here
  // std::cout << "RequestHandler404 initialized." << std::endl;
}

/**
 * handleRequest() - Return reply same as request.
 */
void RequestHandler404::handleRequest(const Request &request_,
                                       Response *response_) noexcept {
    std::cout << "RequestHandler404::handleRequest()" << std::endl;

    response_->result(http::status::not_found);
    response_->version(request_.version());
    response_->set(http::field::content_type, "text/plain");
    response_->body() = "Invalid request type, Handler not found";
    response_->content_length(response_->body().size());
    response_->prepare_payload();
    return;
}


