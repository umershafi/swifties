#include "session.h"
#include "config_parser.h"
#include "logger.h"
#include "request_handler_dispatcher.h"
#include "server.h"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;
namespace http = boost::beast::http;

session::session(boost::asio::io_service &io_service,
                 std::shared_ptr<const RequestHandlerDispatcher> dispatcher,
                 const std::map<std::string, std::string> &credentials,
                 short auth_time)
    : socket_(io_service), dispatcher_(dispatcher), credentials_(credentials),
      auth_time_(auth_time), last_auth_time_(std::chrono::steady_clock::now()) {
  Logger *logger = Logger::getLogger();
  std::cout << "Authorizaiton timeout: " << auth_time_ << "\n";
}

tcp::socket &session::socket() { return socket_; }

void session::start() { handle_read(); }

void session::handle_read() {
  auto self(shared_from_this());
  socket_.async_read_some(
      buffer_.prepare(4096), // Prepare the buffer with a suitable size
      boost::bind(&session::handle_read_callback, this, self,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  Logger *logger = Logger::getLogger();
  logger->logDebugFile("Waiting for data from client");
}

void session::handle_write() {
  auto self(shared_from_this());
  http::async_write(socket_, response_,
                    boost::bind(&session::handle_write_callback, this, self,
                                boost::placeholders::_1,
                                boost::placeholders::_2));
  Logger *logger = Logger::getLogger();
  logger->logDebugFile("Writing response to client");
}

int session::handle_read_callback(std::shared_ptr<session> self,
                                  boost::system::error_code error,
                                  std::size_t bytes_transferred) {
  Logger *logger = Logger::getLogger();
  if (!error) {
    buffer_.commit(bytes_transferred); // Ensure the data is ready for reading

    try {
      // Parse the incoming HTTP request
      http::request_parser<http::string_body> parser;
      parser.put(buffer_.data(), error);
      if (error) {
        logger->logErrorFile("Error parsing request: " + error.message());
        response_ =
            http::response<http::string_body>{http::status::bad_request, 11};
        response_.body() = "Bad request";
        response_.prepare_payload();
        handle_write();
        return 1;
      }

      if (parser.is_done()) {
        auto request = parser.release();
        logger->logTraceHTTPrequest(request, socket_);

        // Log if an Authorization header is set
        auto auth_header_it = request.find(http::field::authorization);
        if (auth_header_it != request.end()) {
          logger->logDebugFile("Authorization header is set: " +
                               request[http::field::authorization].to_string());
        } else {
          logger->logDebugFile("Authorization header is not set");
        }

        // Check if the session is expired
        if (is_session_expired()) {
          // Clear the Authorization header if session is expired
          request.set(http::field::authorization, "");
          logger->logDebugFile(
              "Authorization header is set (after removing): " +
              request[http::field::authorization].to_string());
          logger->logDebugFile(
              "Authorization header removed due to expired session");
          send_unauthorized_response();
          return 1;
        }

        // Check for Authorization header again after potential session
        // expiration
        auth_header_it = request.find(http::field::authorization);
        if (auth_header_it == request.end()) {
          send_unauthorized_response();
          return 1;
        }

        std::string auth_header = auth_header_it->value().to_string();

        if (!authenticate(auth_header)) {
          send_unauthorized_response();
          return 1;
        }

        // Retrieve the appropriate handler based on the request's target URI
        auto target = request.target();
        std::string target_string(target.data(), target.size());
        auto handler = dispatcher_->getRequestHandler(target_string);
        std::string handlerTag = "Handler not found";
        if (!handler) {
          logger->logErrorFile("No handler found for URI: " + target_string);
          response_ =
              http::response<http::string_body>{http::status::not_found, 11};
          response_.body() = "Not Found";
          response_.prepare_payload();
        } else {
          handlerTag = handler->getName();
          handler->handleRequest(request, &response_);
        }
        logger->logDebugFile("Sending a response message to client...");
        logger->logDebugFile("Status Code: " + std::to_string(static_cast<int>(
                                                   response_.result())));
        logger->logResponse(
            handlerTag + " " +
            std::to_string(static_cast<int>(response_.result())));
        handle_write();
        return 0;
      }

      // Not done reading, continue to read more
      handle_read();
    } catch (...) {
      logger->logErrorFile("Exception caught in handle_read_callback");
      response_ = http::response<http::string_body>{
          http::status::internal_server_error, 11};
      response_.body() = "Internal Server Error";
      response_.prepare_payload();
      handle_write();
      return 1;
    }
  } else {
    logger->logErrorFile("Read error: " + error.message());
    return 1;
  }
}

int session::handle_write_callback(std::shared_ptr<session> self,
                                   boost::system::error_code error,
                                   std::size_t) {
  Logger *logger = Logger::getLogger();
  if (!error) {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(tcp::socket::shutdown_both, ignored_ec);
    logger->logDebugFile("Session Complete");
    return 1;
  }
  logger->logErrorFile("Error passed to handle_write_callback: " +
                       error.message());
  return 0;
}

// Base64 decoder function
std::string base64_decode(const std::string &in) {
  std::string out;
  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++)
    T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] =
        i;
  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1)
      break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

// Function which authenticates a user based on the contents of auth_header
bool session::authenticate(const std::string &auth_header) {
  const std::string prefix = "Basic ";
  if (auth_header.compare(0, prefix.size(), prefix) != 0) {
    return false;
  }

  std::string encoded = auth_header.substr(prefix.size());
  std::string decoded = base64_decode(encoded);
  auto delimiter_pos = decoded.find(':');
  if (delimiter_pos == std::string::npos) {
    return false;
  }

  std::string username = decoded.substr(0, delimiter_pos);
  std::string password = decoded.substr(delimiter_pos + 1);

  auto it = credentials_.find(username);
  if (it != credentials_.end() && it->second == password) {
    // Update the last authentication time
    last_auth_time_ = std::chrono::steady_clock::now();
    return true;
  }

  return false;
}

bool session::is_session_expired() {
  auto now = std::chrono::steady_clock::now();
  Logger *logger = Logger::getLogger();
  bool expired =
      std::chrono::duration_cast<std::chrono::seconds>(now - last_auth_time_)
          .count() > auth_time_;
  if (expired) {
    Logger *logger = Logger::getLogger();
    logger->logDebugFile("Session expired, starting new session...");

    // Start a new session
    last_auth_time_ = now;
    handle_read();
  }
  return expired;
}

// Construct unauthorized response for invalid credentials case
void session::send_unauthorized_response() {
  Logger *logger = Logger::getLogger();
  logger->logDebugFile("Sending 401 Unauthorized response");

  // Construct response body
  response_ = http::response<http::string_body>{http::status::unauthorized, 11};
  response_.set(http::field::server, "Beast");
  response_.set(http::field::content_type, "text/html");
  // Should prompt user for credentials if none in header
  response_.set(http::field::www_authenticate,
                "Basic realm=\"User Visible Realm\"");
  response_.body() = "Unauthorized";
  response_.prepare_payload();
  logger->logResponse(
            "Unauthorized " +
            std::to_string(static_cast<int>(response_.result())));
  handle_write();
}