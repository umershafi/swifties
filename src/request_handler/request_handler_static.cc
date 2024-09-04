// request_handler_static.cc
#include <iostream>
#include <fstream>
#include <boost/beast/http.hpp>
#include <boost/filesystem.hpp>
#include "request_handler_static.h"
#include "../http/mime_types.h"

namespace http = boost::beast::http;

std::string RequestHandlerStatic::getName() noexcept {
    return "StaticHandler";
}
/**
 * Constructor - If no root in config string, use "/" as the default.
 */
RequestHandlerStatic::RequestHandlerStatic(const std::string rootString, const PathUri &prefix_)
    : prefix(prefix_), root("/") {
    root = rootString;
    // for (const auto &statement : config.statements_) {
    //     if (statement->tokens_[0] == "root" && statement->tokens_.size() == 2) {
    //         root = statement->tokens_[1];
    //     }
    // }
}

/**
 * handleRequest() - Fill response with static files.
 */
void RequestHandlerStatic::handleRequest(const Request &request_, Response *response_) noexcept {
    // Substitute matched prefix with root
    std::string uri = std::string(request_.target());
    uri.replace(0, prefix.length(), root);
    uri.replace(0, 1, "../"); // Change to relative path
    std::cout << "RequestHandlerStatic::handleRequest() Serving file: " << uri << std::endl;

    // Serve file
    boost::filesystem::path boost_path(uri);
    if (!boost::filesystem::exists(uri) || !boost::filesystem::is_regular_file(uri)) {
        response_->result(http::status::not_found);
        response_->version(request_.version());
        response_->set(http::field::content_type, "text/plain");
        response_->body() = "File not found";
        response_->prepare_payload();
        return;
    }

    std::ifstream f(uri.c_str(), std::ios::in | std::ios::binary);
    if (!f) {
        response_->result(http::status::not_found);
        response_->version(request_.version());
        response_->set(http::field::content_type, "text/plain");
        response_->body() = "File not found";
        response_->prepare_payload();
        return;
    }

    // Read file
    std::string body;
    char c;
    while (f.get(c)) body += c;
    f.close();

    // Use extension to get MIME types
    std::string extension;
    size_t cursor = uri.find_last_of(".");
    if (cursor != std::string::npos) {
        extension = uri.substr(cursor + 1);
    }

    response_->result(http::status::ok);
    response_->version(request_.version());
    response_->set(http::field::content_length, std::to_string(body.length()));
    response_->set(http::field::content_type, mime_types::extension_to_type(extension));
    response_->body() = body;
    response_->prepare_payload();
}
