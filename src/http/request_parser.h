// request_parser.h
#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <boost/beast/http.hpp>
#include <string>

class RequestParser {
public:
    using Request = boost::beast::http::request<boost::beast::http::string_body>;

    RequestParser();
    bool parse(const std::string &raw_request);
    const Request &getRequest() const;

private:
    Request req_;
};

#endif // REQUEST_PARSER_H
