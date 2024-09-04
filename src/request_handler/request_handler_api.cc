// request_handler_api.cc
#include "request_handler_api.h"
#include "../api/crud_handler.h"
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>


namespace http = boost::beast::http;

std::string RequestHandlerAPI::getName() noexcept {
    return "APIHandler";
}
/**
 * handleRequest() - Fill response with static files.
 */

RequestHandlerAPI::RequestHandlerAPI(ICRUDHandler *crud_handler,
                                     const std::string &prefix)
    : crud_handler_(crud_handler), prefix_(prefix) {
  // std::cout << "RequestHandlerAPI initialized with config." << std::endl;
}

void RequestHandlerAPI::handleRequest(const Request &req,
                                      Response *res) noexcept {
  res->version(req.version());
  res->set(http::field::content_type, "application/json");
  // CREATE and GET methods are implemented right now
  // TODO: add other methods
  if (req.method() == http::verb::post) {
    std::string target = std::string(req.target());
    std::string entity = target.substr(target.find_last_of('/') + 1);
    std::string response_body = crud_handler_->create(entity, req.body());
    res->result(http::status::ok);
    res->body() = response_body;
  } else if (req.method() == http::verb::get) {
    std::string target = std::string(req.target());
    std::string entity_id;
    bool success = removePrefix(target, entity_id);

    if (!success) {
      res->result(http::status::not_found);
      res->body() = "Invalid Request: get request";
      res->prepare_payload();
      return;
    }
    std::string entity = entity_id.substr(0, entity_id.find_last_of('/'));
    std::string id_str = entity_id.substr(entity_id.find_last_of('/') + 1);
    std::cout << "id_str: " << id_str << std::endl;

    // assumes entity cannot start with digit
    if (!std::isdigit(id_str[0])) {
      // list request
      std::string response_body = "[";
      int id = 1;
      while (crud_handler_->exists(entity_id, id)) {
        if (id > 1) {
          response_body += ", ";
        }
        response_body += std::to_string(id);
        id++;
      }
      response_body += "]";
      res->body() = response_body;
    } else {
      // read request
      int id;

      // try to convert id to int, return early if invalid id
      try {
        id = boost::lexical_cast<int>(id_str);
      } catch (boost::bad_lexical_cast) {
        res->result(http::status::not_found);
        res->body() = "Invalid Request: retrieve failed";
        res->prepare_payload();
        return;
      }
      // check if the requested file exists in the directory path
      bool exists = crud_handler_->exists(entity, id);
      if (!exists) {
        res->result(http::status::not_found);
        res->body() = "Invalid Request: file not found";
        res->prepare_payload();
        return;
      }
      std::string response_body = crud_handler_->read(entity, id);
      res->body() = response_body;
    }
    res->result(http::status::ok);
  } else if (req.method() == http::verb::put) {
    std::string target = std::string(req.target());
    std::string entity_id;
    bool success = removePrefix(target, entity_id);

    if (!success) {
      res->result(http::status::not_found);
      res->body() = "Invalid Request";
      res->prepare_payload();
      return;
    }
    std::string entity = entity_id.substr(0, entity_id.find_last_of('/'));
    std::string id_str = entity_id.substr(entity_id.find_last_of('/') + 1);

    int id;

    // try to convert id to int, return early if invalid id
    try {
      id = boost::lexical_cast<int>(id_str);
    } catch (boost::bad_lexical_cast) {
      res->result(http::status::not_found);
      res->body() = "Invalid Request";
      res->prepare_payload();
      return;
    }

    success = crud_handler_->update(entity, id, req.body());
    if (!success) {
      res->result(http::status::not_found);
      res->body() = "Invalid Request";
      res->prepare_payload();
      return;
    }
    res->result(http::status::ok);
  } else if (req.method() == http::verb::delete_) {
    std::string target = std::string(req.target());
    std::string entity_id;
    bool success = removePrefix(target, entity_id);

    if (!success) {
      // detailed response message for developers
      res->result(http::status::not_found);
      res->body() = "Invalid Request: entity does not exist";
      res->prepare_payload();
      return;
    }
    std::string entity = entity_id.substr(0, entity_id.find_last_of('/'));
    std::string id_str = entity_id.substr(entity_id.find_last_of('/') + 1);

    int id;

    // try to convert id to int, return early if invalid id
    try {
      id = boost::lexical_cast<int>(id_str);
    } catch (boost::bad_lexical_cast) {
      res->result(http::status::bad_request);
      res->body() = "Invalid Request: entity_id could not be parsed";
      res->prepare_payload();
      return;
    }

    success = crud_handler_->delete_(entity, id);
    // if entity/id does not exist in file path
    if (!success) {
      res->result(http::status::not_found);
      res->body() = "Invalid Request: entity id does not exist";
      res->prepare_payload();
      return;
    }
    res->result(http::status::ok);
    res->body() = "successfully deleted";
  } else {
    res->result(http::status::bad_request);
    res->body() = "{\"error\": \"Unsupported HTTP method\"}";
  }

  res->prepare_payload();
}

bool RequestHandlerAPI::removePrefix(const std::string path,
                                     std::string &new_path) {
  std::cout << "remove prefix" << std::endl;
  std::cout << path << std::endl;
  std::cout << prefix_ << std::endl;
  // invalid paths
  if (path.length() < prefix_.length()) {
    return false;
  } else if (path.substr(0, prefix_.length()) != prefix_) {
    return false;
  }

  new_path = path.substr(prefix_.length());
  return true;
}