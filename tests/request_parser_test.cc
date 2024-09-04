#include "../src/http/request_parser.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <gtest/gtest.h>
#include <iostream>

namespace http = boost::beast::http;

class RequestParserTest : public ::testing::Test {
protected:
  RequestParser
      parser; // Updated to use RequestParser instead of http::request_parser
  boost::beast::flat_buffer buffer;
  boost::system::error_code ec;

  void SetUp() override {
    // Initialize parser before each test
    parser = RequestParser();
  }

  // Other test methods...
};

TEST_F(RequestParserTest, WrongProtocolFirstChar) {
  const std::string input =
      "GET / MMTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, BadVersionMinor) {
  const std::string input =
      "GET / HTTP/1.1a\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, IndeterminateRequest) {
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, SpacedRequest) {
  const std::string input = "GET / HTTP /1 . 1\r\n  Host: www.example.com "
                            "\r\n\t  Type: test\r\n Connection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, SpacedOneRequest) {
  const std::string input =
      "GET / HTTP /1 . 1\r\n  Host: www.example.com \r\n\t\r\n  Type: test\r\n "
      "Connection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, IncompleteRequest) {
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, InvalidHeaderFormat) {
  const std::string input =
      "GET / HTTP/1.1\r\nInvalidHeaderWithoutColon\r\nConnection: "
      "close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, EmptyRequest) {
  const std::string input = "";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail, so the return value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, GoodRequest) {
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to succeed, so the return value should be true
  EXPECT_TRUE(parsingResult);

  // Additional checks can be added to verify the parsed request if needed
}

TEST_F(RequestParserTest, ParsingSuccess) {
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to succeed, so the return value should be true
  EXPECT_TRUE(parsingResult);
}

TEST_F(RequestParserTest, ParsingFailureWithError) {
  const std::string input =
      "GET / HTTP/1.1\r\nInvalidHeaderWithoutColon\r\nConnection: "
      "close\r\n\r\n";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail due to an error, so the return value should be
  // false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, ParsingFailureIncompleteRequest) {
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail due to an incomplete request, so the return
  // value should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, ParsingFailureEmptyRequest) {
  const std::string input = "";

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(input);

  // Expect the parsing to fail due to an empty request, so the return value
  // should be false
  EXPECT_FALSE(parsingResult);
}

TEST_F(RequestParserTest, ParsingFailureExceptionThrown) {
  // Create a request with invalid format causing an exception
  const std::string input =
      "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";
  // Modify input to cause an exception
  std::string modifiedInput =
      input.substr(0, 10); // Creating invalid request to cause an exception

  // Call the parse method directly and check its return value
  bool parsingResult = parser.parse(modifiedInput);

  // Expect the parsing to fail due to an exception, so the return value should
  // be false
  EXPECT_FALSE(parsingResult);
}