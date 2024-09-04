#include "../src/config_parser.h"
#include "../src/request_handler/request_handler_echo.h"
#include "../src/request_handler_dispatcher.h"
#include "../src/session.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <sstream>
#include <thread>

using ::testing::_;
using ::testing::Return;

class MockRequestHandlerDispatcher : public RequestHandlerDispatcher {
public:
  MockRequestHandlerDispatcher(const NginxConfig &config)
      : RequestHandlerDispatcher(config) {}
  MOCK_METHOD1(getRequestHandler,
               std::shared_ptr<RequestHandler>(const std::string &target));
};

class SessionTest : public ::testing::Test {
protected:
  NginxConfig config;
  std::shared_ptr<MockRequestHandlerDispatcher> dispatcher;
  boost::asio::io_service io_service;
  std::shared_ptr<session> new_session;
  std::map<std::string, std::string> credentials;
  short auth_time;

  void SetUp() override {
    std::string config_string = R"(
    server {
        port 80;
        timer 10;
        credentials {
            tariq:123;
            milly:456;
            shravan:789;
            umer:101;
        }
        location /static/ StaticHandler {
            root /data;
        }
        location /echo/ EchoHandler {   
        }
        location /api/ APIHandler {
            root ./api_storage/;
        }
        location /health HealthHandler {
        }
        location /sleep SleepHandler {
        }
    }
    )";

    // Parse the configuration string
    NginxConfigParser config_parser;
    std::istringstream config_stream(config_string);
    config_parser.Parse(&config_stream, &config);

    auth_time = config.get_auth_time();
    // Extract credentials from the configuration
    credentials = config.get_credentials();

    dispatcher = std::make_shared<MockRequestHandlerDispatcher>(config);
    new_session =
        std::make_shared<session>(io_service, dispatcher, credentials, auth_time);
  }

  void TearDown() override {
    // Clean up if needed
  }

  void simulate_read_data(session &sess, const std::string &data) {
    auto &buffer = sess.buffer_;
    buffer.commit(boost::asio::buffer_copy(buffer.prepare(data.size()),
                                           boost::asio::buffer(data)));
  }
};

TEST_F(SessionTest, GoodRequestWithAuthorization) {
  std::shared_ptr<RequestHandler> empty_handler =
      std::make_shared<RequestHandlerEcho>();
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(empty_handler));

  // Simulate a good HTTP request with valid Authorization header
  new_session->start();
  simulate_read_data(*new_session,
                     "GET / HTTP/1.1\r\nAuthorization: Basic "
                     "dGFyaXE6MTIz\r\n\r\n"); // Base64 for "tariq:123"
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 0);
}

TEST_F(SessionTest, UnauthorizedRequestWithoutHeader) {
  std::shared_ptr<RequestHandler> empty_handler =
      std::make_shared<RequestHandlerEcho>();
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(empty_handler));

  // Simulate a HTTP request without Authorization header
  new_session->start();
  simulate_read_data(*new_session, "GET / HTTP/1.1\r\n\r\n");
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 1);
}

TEST_F(SessionTest, UnauthorizedRequestWithInvalidHeader) {
  std::shared_ptr<RequestHandler> empty_handler =
      std::make_shared<RequestHandlerEcho>();
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(empty_handler));

  // Simulate a HTTP request with an invalid Authorization header
  new_session->start();
  simulate_read_data(*new_session,
                     "GET / HTTP/1.1\r\nAuthorization: Basic "
                     "aW52YWxpZDppbnZhbGlk\r\n\r\n"); // Base64 for
                                                      // "invalid:invalid"
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 1);
}

TEST_F(SessionTest, HandlerNotFound) {
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(nullptr)); // No handler found

  // Simulate a HTTP request with a valid Authorization header
  new_session->start();
  simulate_read_data(
      *new_session,
      "GET /nonexistent HTTP/1.1\r\nAuthorization: Basic dGFyaXE6MTIz\r\n\r\n");
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 0);
}

TEST_F(SessionTest, InternalServerError) {
  std::shared_ptr<RequestHandler> empty_handler =
      std::make_shared<RequestHandlerEcho>();
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(empty_handler));

  // Simulate an exception in handle_read_callback by sending incomplete data
  new_session->start();
  simulate_read_data(
      *new_session,
      "GET / HTTP/1.1\r\nAuthorization: Basic dGFyaXE6MTIz"); // Missing
                                                              // \r\n\r\n
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 1);
}

TEST_F(SessionTest, WriteCallback) {
  std::shared_ptr<session> mock_session = new_session;

  // Simulate handling a write callback
  int ret = mock_session->handle_write_callback(
      mock_session->shared_from_this(),
      boost::system::error_code(), // No error
      0);                          // Provide a placeholder size

  // Assert that the function returns 1 to indicate successful write callback
  EXPECT_EQ(ret, 1);
}

TEST_F(SessionTest, SessionExpiration) {
  std::shared_ptr<RequestHandler> empty_handler =
      std::make_shared<RequestHandlerEcho>();
  EXPECT_CALL(*dispatcher, getRequestHandler(_))
      .WillRepeatedly(Return(empty_handler));

  // Simulate the first good HTTP request with valid Authorization header
  new_session->start();
  simulate_read_data(*new_session,
                     "GET / HTTP/1.1\r\nAuthorization: Basic "
                     "dGFyaXE6MTIz\r\n\r\n"); // Base64 for "tariq:123"
  int ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);
  EXPECT_EQ(ret, 0);

  // Wait for 11 seconds to simulate session expiration
  std::this_thread::sleep_for(std::chrono::seconds(11));

  // Simulate the second request which should now be unauthorized
  simulate_read_data(*new_session,
                     "GET / HTTP/1.1\r\nAuthorization: Basic "
                     "dGFyaXE6MTIz\r\n\r\n"); // Base64 for "tariq:123"
  ret = new_session->handle_read_callback(
      new_session, boost::system::error_code(), 4096);

  EXPECT_EQ(ret, 1);
}
