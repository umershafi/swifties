// request_parser.cc
#include "request_parser.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>

RequestParser::RequestParser() {}

bool RequestParser::parse(const std::string &raw_request) {
    try {
        boost::beast::flat_buffer buffer;
        // Copy the raw request data into the buffer
        buffer.commit(boost::asio::buffer_copy(buffer.prepare(raw_request.size()), boost::asio::buffer(raw_request)));

        // Create a request parser
        boost::beast::http::request_parser<boost::beast::http::string_body> parser;

        // Parse the buffer directly
        boost::beast::error_code ec;
        auto const bytes_parsed = parser.put(boost::asio::buffer(buffer.data()), ec);
        buffer.consume(bytes_parsed);

        // Check if parsing was successful
        if (ec) {
            std::cerr << "Request parsing error: " << ec.message() << std::endl;
            return false;
        }

        if (parser.is_done()) {
            req_ = parser.release();
            return true;
        } else {
            std::cerr << "Request parsing incomplete." << std::endl;
            return false;
        }
    } catch (const std::exception &e) {
        std::cerr << "Request parsing error: " << e.what() << std::endl;
        return false;
    }
}

const RequestParser::Request &RequestParser::getRequest() const {
    return req_;
}
