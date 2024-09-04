#include "../src/config_parser.h"
#include "../src/server.h"
#include "../src/session.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

class ServerTest : public ::testing::Test {
protected:
  NginxConfigParser parser_;
  NginxConfig out_config_;

  bool parseString(const std::string config_string) {
    std::stringstream config_stream(config_string);
    return parser_.Parse(&config_stream, &out_config_);
  }
};

// Define a mock session class to simulate an incoming connection
class MockSession : public session {
public:
  MockSession(boost::asio::io_service &io_service,
              std::shared_ptr<const RequestHandlerDispatcher> dispatcher, const std::map<std::string, std::string> &credentials, short auth_time)
      : session(io_service, dispatcher, credentials, auth_time) {}
  MOCK_METHOD0(start, void());
};

TEST_F(ServerTest, HandleAcceptTest) {
  // Initialize the io_service
  boost::asio::io_service io_service;

  // Create a mock session with the io_service and dispatcher
  std::shared_ptr<MockSession> mock_session =
      std::make_shared<MockSession>(io_service, nullptr, std::map<std::string, std::string>(), 10);

  // Create a server object
  server s(io_service, 8080, out_config_, std::map<std::string, std::string>(), 10);

  // Simulate an incoming connection
  tcp::socket mock_socket(io_service);
  tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"),
                         8080);
  mock_socket.connect(endpoint);

  // Call the handle_accept function manually
  s.handle_accept(mock_session, boost::system::error_code());

  // Run the io_service to process the asynchronous accept operation
  // Close the mock socket
  mock_socket.close();
}
