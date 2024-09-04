#include "../src/config_parser.h"
#include "../src/request_handler/request_handler_404.h"
#include "../src/request_handler/request_handler_api.h"
#include "../src/request_handler/request_handler_echo.h"
#include "../src/request_handler/request_handler_health.h"
#include "../src/request_handler/request_handler_static.h"
#include "../src/request_handler_dispatcher.h"
#include "gtest/gtest.h"

class RequestHandlerDispatcherTest : public ::testing::Test {
protected:
  NginxConfig empty_config;
  RequestHandlerDispatcher *dispatcher;

  void SetUp() override {
    dispatcher = new RequestHandlerDispatcher(empty_config);
  }

  void TearDown() override { delete dispatcher; }

  NginxConfig parseConfig(const std::string &config_text) {
    NginxConfigParser parser;
    NginxConfig config;
    std::stringstream config_stream(config_text);
    parser.Parse(&config_stream, &config);
    return config;
  }
};

TEST_F(RequestHandlerDispatcherTest, GetRequestHandlerRoot) {
  auto handler = dispatcher->getRequestHandler("/");
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(typeid(*handler), typeid(RequestHandler404));
}

TEST_F(RequestHandlerDispatcherTest, RegisterPathEchoHandler) {
  NginxConfig config = parseConfig("location /echo EchoHandler {}");
  dispatcher->registerPath("/echo", "EchoHandler", config);

  auto handler = dispatcher->getRequestHandler("/echo");
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(typeid(*handler), typeid(RequestHandlerEcho));
}

TEST_F(RequestHandlerDispatcherTest, RegisterPathStaticHandler) {
  NginxConfig config =
      parseConfig("location /static StaticHandler { root /www; }");
  dispatcher->registerPath("/static", "StaticHandler", config);

  auto handler = dispatcher->getRequestHandler("/static");
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(typeid(*handler), typeid(RequestHandlerStatic));
}

TEST_F(RequestHandlerDispatcherTest, RegisterPathAPIHandler) {
  NginxConfig config =
      parseConfig("location /api APIHandler { root /api_root; }");
  dispatcher->registerPath("/api", "APIHandler", config);

  auto handler = dispatcher->getRequestHandler("/api");
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(typeid(*handler), typeid(RequestHandlerAPI));
}

TEST_F(RequestHandlerDispatcherTest, RegisterPathHealthHandler) {
  NginxConfig config = parseConfig("location /health HealthHandler {}");
  dispatcher->registerPath("/health", "HealthHandler", config);

  auto handler = dispatcher->getRequestHandler("/health");
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(typeid(*handler), typeid(RequestHandlerHealth));
}

// TEST_F(RequestHandlerDispatcherTest, RegisterPathSleepHandler) {
//   NginxConfig config = parseConfig("location /sleep SleepHandler {}");
//   dispatcher->registerPath("/sleep", "SleepHandler", config);

//   auto handler = dispatcher->getRequestHandler("/sleep");
//   EXPECT_NE(handler, nullptr);
//   EXPECT_EQ(typeid(*handler), typeid(RequestHandlerSleep));
// }

TEST_F(RequestHandlerDispatcherTest, InitRequestHandlers) {
  NginxConfig config =
      parseConfig("server {"
                  "  location /echo EchoHandler {}"
                  "  location /static StaticHandler { root /www; }"
                  "  location /api APIHandler { root /api_root; }"
                  "  location /health HealthHandler {}"
                  "}");

  dispatcher->initRequestHandlers(config);

  EXPECT_EQ(typeid(*dispatcher->getRequestHandler("/echo")),
            typeid(RequestHandlerEcho));
  EXPECT_EQ(typeid(*dispatcher->getRequestHandler("/static")),
            typeid(RequestHandlerStatic));
  EXPECT_EQ(typeid(*dispatcher->getRequestHandler("/api")),
            typeid(RequestHandlerAPI));
  EXPECT_EQ(typeid(*dispatcher->getRequestHandler("/health")),
            typeid(RequestHandlerHealth));
  EXPECT_EQ(typeid(*dispatcher->getRequestHandler("/unknown")),
            typeid(RequestHandler404));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
