#include "../src/logger.h"
#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

class LoggerTest : public ::testing::Test {
protected:
    Logger* logger;

    void SetUp() override {
        logger = new Logger();
    }

    void TearDown() override {
        delete logger;
    }
};

TEST_F(LoggerTest, LogServerInitialization) {
    EXPECT_NO_THROW(logger->logServerInitialization());
}

TEST_F(LoggerTest, LogTraceFile) {
    std::string message = "Test trace message";
    EXPECT_NO_THROW(logger->logTraceFile(message));
}

TEST_F(LoggerTest, LogErrorFile) {
    std::string message = "Test error message";
    EXPECT_NO_THROW(logger->logErrorFile(message));
}

TEST_F(LoggerTest, LogDebugFile) {
    std::string message = "Test debug message";
    EXPECT_NO_THROW(logger->logDebugFile(message));
}

TEST_F(LoggerTest, LogWarningFile) {
    std::string message = "Test warning message";
    EXPECT_NO_THROW(logger->logWarningFile(message));
}

TEST_F(LoggerTest, LogSig) {
    EXPECT_NO_THROW(logger->logSig());
}

TEST_F(LoggerTest, LogTrace) {
    EXPECT_NO_THROW(logger->logTrace());
}

TEST_F(LoggerTest, LogDebug) {
    EXPECT_NO_THROW(logger->logDebug());
}

TEST_F(LoggerTest, LogWarning) {
    EXPECT_NO_THROW(logger->logWarning());
}

TEST_F(LoggerTest, LogError) {
    EXPECT_NO_THROW(logger->logError());
}

TEST_F(LoggerTest, LogFatal) {
    EXPECT_NO_THROW(logger->logFatal());
}

TEST_F(LoggerTest, LogTraceHTTPRequest) {
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/", 11};
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);
    EXPECT_NO_THROW(logger->logTraceHTTPrequest(req, socket));
}

