#ifndef REQUEST_HANDLER_API_H
#define REQUEST_HANDLER_API_H

#include <boost/beast/http.hpp>
#include "request_handler.h"
#include "../config_parser.h"
#include "../api/crud_handler.h"

class RequestHandlerAPI : public RequestHandler {
public:
    std::string getName() noexcept override;
    // data_path parameter specifies root directory of the referenced data
    RequestHandlerAPI(ICRUDHandler* crud_handler, const std::string &prefix);

    void handleRequest(const Request &request_, Response *response_) noexcept override;
private:
    ICRUDHandler* crud_handler_;
    std::string prefix_;

    // helper function to remove api prefix from path and put result in new_path, returns whether prefix_ is a substring of path
    // and if path was able to successfully remove the prefix
    bool removePrefix(const std::string path, std::string& new_path);
};

#endif // REQUEST_HANDLER_API_H