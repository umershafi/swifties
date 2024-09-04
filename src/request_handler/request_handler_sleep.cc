#include "request_handler_sleep.h"
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <thread>   // For std::this_thread::sleep_for
#include <chrono>   // For std::chrono::seconds
#include <iostream>

namespace http = boost::beast::http;

std::string RequestHandlerSleep::getName() noexcept {
    return "SleepHandler";
}

/**
 * Constructor - Initialize the echo handler.
 */
RequestHandlerSleep::RequestHandlerSleep() {
  // Optionally, process the config here
  std::cout << "RequestHandlerSleep initialized with config." << std::endl;
}

/**
 * handleRequest() - Return reply same as request.
 */
void RequestHandlerSleep::handleRequest(const Request &request_,
                                       Response *response_) noexcept {
  // Log the incoming request (optional, for debug purposes)
    std::cout << "Received request at /sleep, processing with delay..." << std::endl;

    // Simulate processing time by sleeping
    std::this_thread::sleep_for(std::chrono::seconds(5)); // Sleep for 5 seconds

    // Set up the response after the delay
    response_->result(http::status::ok); // Set HTTP response status to 200 OK
    response_->version(request_.version()); // Echo back the HTTP version from the request
    response_->set(http::field::content_type, "text/plain"); // Set the Content-Type of the response
    response_->body() = "Processed after a delay"; // Response body content
    response_->prepare_payload(); // Prepare the payload, which calculates Content-Length and other necessary headers

    // Log completion of request handling (optional)
    std::cout << "Request processed and response sent after delay." << std::endl;
}
