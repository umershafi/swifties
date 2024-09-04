#include "../src/http/request_parser.h"
#include "../src/request_handler/request_handler_404.h"
#include "../src/request_handler/request_handler_api.h"
#include "../src/request_handler/request_handler_echo.h"
#include "../src/request_handler/request_handler_health.h"
#include "../src/request_handler/request_handler_sleep.h"
#include "../src/request_handler/request_handler_static.h"
#include <boost/asio/buffer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <type_traits>

namespace http = boost::beast::http;

// use a hashmap to mock a filesystem
class MockCRUDHandler : public ICRUDHandler {
private:
  std::unordered_map<std::string, std::string> mock_filesystem;
  std::string getPath(const std::string &entity, int id) {
    return entity + "/" + std::to_string(id);
  }

  int getNextId(const std::string &entity) {
    int id = 1;
    // get next available id
    while (mock_filesystem.find(getPath(entity, id)) != mock_filesystem.end()) {
      ++id;
    }
    return id;
  }

public:
  MockCRUDHandler() {}

  bool exists(const std::string &entity, int id) override {
    if (mock_filesystem.find(getPath(entity, id)) == mock_filesystem.end()) {
      return false;
    } else {
      return true;
    }
  }

  std::string create(const std::string &entity,
                     const std::string &data) override {
    int id = getNextId(entity);
    std::string path = getPath(entity, id);
    mock_filesystem[path] = data;

    return "{\"id\": " + std::to_string(id) + "}";
  }

  std::string read(const std::string &entity, int id) override {
    std::string path = getPath(entity, id);
    return mock_filesystem[path];
  }

  bool update(const std::string &entity, int id,
              const std::string &data) override {
    if (!exists(entity, id)) {
      return false;
    }
    std::string path = getPath(entity, id);
    mock_filesystem[path] = data;
    return true;
  }

  bool delete_(const std::string &entity, int id) override {
    if (!exists(entity, id)) {
      return false;
    }
    std::string path = getPath(entity, id);
    auto iter = mock_filesystem.find(path);
    mock_filesystem.erase(iter);
    return true;
  }
};

class RequestHandlerTest : public ::testing::Test {
private:
  const std::string TEST_API_STORAGE_PREFIX = "/api/";

protected:
  RequestHandler404 handler_404;
  RequestHandlerEcho handler_echo;
  RequestHandlerStatic
      handler_static; // Add RequestHandlerStatic to the fixture
  RequestHandlerAPI handler_api;
  RequestHandlerHealth handler_health;
  RequestHandlerSleep handler_sleep;
  RequestParser request_parser;

public:
  // Default constructor
  RequestHandlerTest()
      : handler_static("/data/", "/static/"),
        handler_api(new MockCRUDHandler(), TEST_API_STORAGE_PREFIX) {}

  void SetUp() override {
    // Initialize objects before each test
    request_parser = RequestParser();
  }
};

// Test case to verify handling of 404 request
TEST_F(RequestHandlerTest, NotFoundRequestHandling) {
  // Create a valid request for 404 handler
  const std::string valid_request_str =
      "GET /static/hello.txt HTTP/1.1\r\nHost: www.example.com\r\nConnection: "
      "close\r\n\r\n";
  http::request<http::string_body> valid_request;

  // Parse the valid request
  http::request<http::string_body> *valid_request_ptr = &valid_request;
  request_parser.parse(valid_request_str);

  handler_404.getName();

  // Create a response object to capture the handler's response for 404 handler
  http::response<http::string_body> response_404;
  handler_404.handleRequest(valid_request, &response_404);
  std::cout << valid_request << std::endl;
  // Verify that response status code is not_found
  EXPECT_EQ(response_404.result_int(), 404);
  // Verify content type is text/plain
  EXPECT_EQ(response_404.find(http::field::content_type)->value(),
            "text/plain");
  // Verify response body
  EXPECT_EQ(response_404.body(), "Invalid request type, Handler not found");
}

// Test case to verify handling of echo request
TEST_F(RequestHandlerTest, EchoRequestHandling) {
  // Create a valid request for echo handler
  const std::string input = "GET /echo HTTP/1.1\r\nHost: "
                            "www.example.com\r\nConnection: close\r\n\r\n";

  // Parse the echo request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
  parser.put(buffer.data(), error);
  auto request = parser.release();
  // Log Handler Name
  handler_echo.getName();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_echo;
  handler_echo.handleRequest(request, &response_echo);
  std::cout << response_echo.body() << std::endl;
  EXPECT_EQ(input, response_echo.body());
}

// Test case to verify handling of static file request
TEST_F(RequestHandlerTest, StaticFileRequestHandlingValid) {
  const std::string input = "GET /static/hello.txt HTTP/1.1\r\nHost: "
                            "www.example.com\r\nConnection: close\r\n\r\n";

  // Parse the echo request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  handler_static.getName();

  http::response<http::string_body> response_echo;
  handler_static.handleRequest(request, &response_echo);
  std::cout << "body: " << response_echo.body() << std::endl;
  EXPECT_EQ("This is CRAZY, it totally works.\n", response_echo.body());
}

// Test case to verify handling of static file request -- VALID CASE
TEST_F(RequestHandlerTest, StaticFileRequestHandlingInvalid) {
  // Create a valid request for static file handler
  // const std::string static_request_str =
  //     "GET /static/hello.txt HTTP/1.1\r\nHost: www.example.com\r\nConnection:
  //     close\r\n\r\n";
  // http::request<http::string_body> static_request;

  // // Parse the static file request
  // http::request<http::string_body>* static_request_ptr = &static_request;
  // request_parser.parse(static_request_str);

  // // Create a response object to capture the handler's response for static
  // file handler http::response<http::string_body> response_static;
  // handler_static.handleRequest(static_request, &response_static);
  const std::string input = "GET /static/hellooooo.txt HTTP/1.1\r\nHost: "
                            "www.example.com\r\nConnection: close\r\n\r\n";

  // Parse the echo request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;
  // http::request<http::string_body> *echo_request_ptr = &echo_request;
  // bool parsingResult = request_parser.parse(echo_request_str);
  // std::cout << request_parser.release() << std::endl;
  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_echo;
  handler_static.handleRequest(request, &response_echo);
  std::cout << "body: " << response_echo.body() << std::endl;
  EXPECT_EQ("File not found", response_echo.body());
}

// Test case to verify creating of file and correct IDs using API
TEST_F(RequestHandlerTest, CRUDAPICreateRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;
  handler_api.getName();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  std::cout << "body: " << response_api.body() << std::endl;
  EXPECT_EQ("{\"id\": 1}", response_api.body());

  std::string input2 =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value2\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ("{\"id\": 2}", response_api2.body());
}

// Test case to verify reading of file using API
TEST_F(RequestHandlerTest, CRUDAPIReadRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);

  std::string input2 =
      "GET /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ("{\"example_key\": \"example_value1\"}", response_api2.body());
}

// Test case to verify reading of bad file using API
TEST_F(RequestHandlerTest, CRUDAPIBadReadRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);

  std::string input2 =
      "GET /ap/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ(response_api2.result(), http::status::not_found);
  EXPECT_EQ(response_api2.body(), "Invalid Request: get request");

  std::string input3 =
      "GET /api/Shoes/1invalid HTTP/1.1\r\nHost: "
      "www.example.com\r\nContent-Type: application/json\r\n\r\n\r\n";

  // Parse the get request
  http::request_parser<http::string_body> parser3;
  boost::system::error_code error3;
  boost::beast::flat_buffer buffer3;
  buffer3.prepare(input3.size());
  boost::asio::buffer_copy(buffer3.prepare(input3.size()),
                           boost::asio::buffer(input3));
  buffer3.commit(input3.size());
  parser3.eager(true);
  parser3.put(buffer3.data(), error3);
  auto request3 = parser3.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api3;
  handler_api.handleRequest(request3, &response_api3);
  std::cout << "body: " << response_api3.body() << std::endl;
  EXPECT_EQ(response_api3.result(), http::status::not_found);
  EXPECT_EQ(response_api3.body(), "Invalid Request: retrieve failed");
}

// Test case to verify update of file using API
TEST_F(RequestHandlerTest, CRUDAPIUpdateRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);

  std::string input2 =
      "PUT /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value2\"}\r\n";

  // Parse the put request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ(response_api2.result(), http::status::ok);

  std::string input3 =
      "GET /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser3;
  boost::system::error_code error3;
  boost::beast::flat_buffer buffer3;
  buffer3.prepare(input3.size());
  boost::asio::buffer_copy(buffer3.prepare(input3.size()),
                           boost::asio::buffer(input3));
  buffer3.commit(input3.size());
  parser3.eager(true);
  parser3.put(buffer3.data(), error3);
  auto request3 = parser3.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api3;
  handler_api.handleRequest(request3, &response_api3);
  std::cout << "body: " << response_api3.body() << std::endl;
  EXPECT_EQ(response_api3.result(), http::status::ok);
  EXPECT_EQ(response_api3.body(), "{\"example_key\": \"example_value2\"}");
}

// Test case to verify update of unexistent file using API
TEST_F(RequestHandlerTest, CRUDAPIBadUpdateRequestHandling) {
  std::string input =
      "PUT /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  std::cout << "body: " << response_api.body() << std::endl;
  EXPECT_EQ(response_api.result(), http::status::not_found);
  EXPECT_EQ(response_api.body(), "Invalid Request");
}

// Test case to verify deletion of file using API
TEST_F(RequestHandlerTest, CRUDAPIDeleteRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for api handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);

  std::string input2 =
      "DELETE /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value2\"}\r\n";

  // Parse the delete request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for api handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ(response_api2.result(), http::status::ok);
}

// Test case to verify deletion of non-existent file using API
TEST_F(RequestHandlerTest, CRUDAPIBadDeleteRequestHandling) {
  std::string input =
      "DELETE /api/Shoes/abc HTTP/1.1\r\nHost: "
      "www.example.com\r\nContent-Type: application/json\r\nContent-Length: "
      "33\r\n\r\n{\"example_key\": \"example_value1\"}\r\n";

  // Parse the delete request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for api handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  std::cout << "body: " << response_api.body() << std::endl;
  EXPECT_EQ(response_api.result(), http::status::bad_request);
  EXPECT_EQ(response_api.body(),
            "Invalid Request: entity_id could not be parsed");

  std::string input2 =
      "DELETE /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the delete request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for api handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ(response_api2.result(), http::status::not_found);
  EXPECT_EQ(response_api2.body(), "Invalid Request: entity id does not exist");
}

// Test case to verify List functionality of entity using API
TEST_F(RequestHandlerTest, CRUDAPIListRequestHandling) {
  std::string input =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: 33\r\n\r\n{\"example_key\": "
      "\"example_value1\"}\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();
  std::cout << request << std::endl;

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);

  std::string input2 =
      "GET /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the get request
  http::request_parser<http::string_body> parser2;
  boost::system::error_code error2;
  boost::beast::flat_buffer buffer2;
  buffer2.prepare(input2.size());
  boost::asio::buffer_copy(buffer2.prepare(input2.size()),
                           boost::asio::buffer(input2));
  buffer2.commit(input2.size());
  parser2.eager(true);
  parser2.put(buffer2.data(), error2);
  auto request2 = parser2.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api2;
  handler_api.handleRequest(request2, &response_api2);
  std::cout << "body: " << response_api2.body() << std::endl;
  EXPECT_EQ("[1]", response_api2.body());

  std::string input3 =
      "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the post request
  http::request_parser<http::string_body> parser3;
  boost::system::error_code error3;
  boost::beast::flat_buffer buffer3;
  buffer3.prepare(input3.size());
  boost::asio::buffer_copy(buffer3.prepare(input3.size()),
                           boost::asio::buffer(input3));
  buffer3.commit(input3.size());
  parser3.eager(true);
  parser3.put(buffer3.data(), error3);
  auto request3 = parser3.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api3;
  handler_api.handleRequest(request3, &response_api3);
  std::cout << "body: " << response_api3.body() << std::endl;

  std::string input4 =
      "GET /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\n\r\n\r\n";

  // Parse the get request
  http::request_parser<http::string_body> parser4;
  boost::system::error_code error4;
  boost::beast::flat_buffer buffer4;
  buffer4.prepare(input4.size());
  boost::asio::buffer_copy(buffer4.prepare(input4.size()),
                           boost::asio::buffer(input4));
  buffer4.commit(input4.size());
  parser4.eager(true);
  parser4.put(buffer4.data(), error4);
  auto request4 = parser4.release();

  // Create a response object to capture the handler's response for echo handler
  http::response<http::string_body> response_api4;
  handler_api.handleRequest(request4, &response_api4);
  std::cout << "body: " << response_api4.body() << std::endl;
  EXPECT_EQ("[1, 2]", response_api4.body());
}

// Test case to verify handling of health request
TEST_F(RequestHandlerTest, HealthRequestHandling) {
  const std::string input = "GET /health HTTP/1.1\r\nHost: "
                            "www.example.com\r\nConnection: close\r\n\r\n";

  // Parse the echo request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.put(buffer.data(), error);
  auto request = parser.release();

  // Log handler name
  handler_health.getName();

  http::response<http::string_body> response_health;
  handler_health.handleRequest(request, &response_health);
  EXPECT_EQ("OK", response_health.body());
}

TEST_F(RequestHandlerTest, SleepRequestHandling) {
  const std::string input = "GET /sleep HTTP/1.1\r\nHost: "
                            "www.example.com\r\nConnection: close\r\n\r\n";

  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.put(buffer.data(), error);
  auto request = parser.release();

  // Log handler name
  handler_sleep.getName();

  http::response<http::string_body> response_sleep;
  handler_sleep.handleRequest(request, &response_sleep);
  EXPECT_EQ("Processed after a delay", response_sleep.body());
}

// NEW

// Test case to verify large data handling in create operation
TEST_F(RequestHandlerTest, CRUDAPICreateLargeDataHandling) {
  std::string large_data(10000,
                         'x'); // Create a large string of 10,000 'x' characters
  std::string input =
      "POST /api/Books HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: " +
      std::to_string(large_data.size()) + "\r\n\r\n{\"content\": \"" +
      large_data + "\"}\r\n";

  // Parse the request
  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();

  // Create a response object to capture the API handler's response
  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  std::cout << "body: " << response_api.body() << std::endl;
  EXPECT_NE(response_api.result(), http::status::internal_server_error);
  EXPECT_EQ(response_api.body(), "{\"id\": 1}");
}

TEST_F(RequestHandlerTest, ExcessivelyLongUriHandling) {
  std::string long_uri(10000, 'a'); // Generate a very long URI
  std::string input =
      "GET /api/" + long_uri + " HTTP/1.1\r\nHost: www.example.com\r\n\r\n";

  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();

  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  EXPECT_TRUE(response_api.result() == http::status::uri_too_long ||
              response_api.result() == http::status::bad_request);
}

TEST_F(RequestHandlerTest, PathCaseSensitivity) {
  std::string input =
      "GET /API/Books/1 HTTP/1.1\r\nHost: www.example.com\r\n\r\n";

  http::request_parser<http::string_body> parser;
  boost::system::error_code error;
  boost::beast::flat_buffer buffer;
  buffer.prepare(input.size());
  boost::asio::buffer_copy(buffer.prepare(input.size()),
                           boost::asio::buffer(input));
  buffer.commit(input.size());
  parser.eager(true);
  parser.put(buffer.data(), error);
  auto request = parser.release();

  http::response<http::string_body> response_api;
  handler_api.handleRequest(request, &response_api);
  EXPECT_EQ(response_api.result(),
            http::status::not_found); // Assuming case-sensitivity that results
                                      // in a "not found" error
}
