#include "../src/config_parser.h"
#include "gtest/gtest.h"
#include <fstream>
// test fixture added
class NginxConfigParserTestFixture : public ::testing::Test {
protected:
  bool ParseString(const std::string &config_string) {
    std::istringstream config_stream(config_string);
    return parser.Parse(&config_stream, &out_config);
  }

  bool ParseFile(const std::string &file_name) {
    return parser.Parse(file_name.c_str(), &out_config);
  }

  NginxConfigParser parser;
  NginxConfig out_config;
};

// single statement
TEST_F(NginxConfigParserTestFixture, OneStatementConfig) {
  EXPECT_TRUE(ParseString("server;"));
  EXPECT_EQ(out_config.statements_.size(), 1);
}

// two statements
TEST_F(NginxConfigParserTestFixture, TwoStatementConfig) {
  EXPECT_TRUE(ParseString("server;  listen 80;"));
  EXPECT_EQ(out_config.statements_.size(), 2);
}

// missing semicolon (invalid statement)cd cs2
TEST_F(NginxConfigParserTestFixture, InvalidConfig) {
  EXPECT_FALSE(ParseString("server \n"));
}

// missing curly close bracket
TEST_F(NginxConfigParserTestFixture, UnevenOpenBracket) {
  EXPECT_FALSE(ParseString("http {\n  server {\n listen 80;\n\r  }\n"));
}

// extra curly close brack
TEST_F(NginxConfigParserTestFixture, UnevenCloseBracket) {
  EXPECT_FALSE(ParseString("http {\n  server {\n listen 80;\t\n  }\n}}"));
}

// nested matching brackets
TEST_F(NginxConfigParserTestFixture, EvenBracket) {
  EXPECT_TRUE(ParseString("http {\n  server {\n    listen 80;\n  }\n}"));
}

// get port number
TEST_F(NginxConfigParserTestFixture, GetPortNumber) {
  bool success = ParseString("foo bar; port 80;");

  int port = out_config.get_config_port();
  EXPECT_TRUE(port == 80);
}

// invalid port number
TEST_F(NginxConfigParserTestFixture, InvalidPort) {
  bool success =
      parser.Parse("./config_parser_tests/invalid_port", &out_config);

  int port = out_config.get_config_port();
  EXPECT_TRUE(port == -1);
}

// nested port number
TEST_F(NginxConfigParserTestFixture, NestedPort) {
  bool success = parser.Parse("./config_parser_tests/nested_port", &out_config);

  int port = out_config.get_config_port();
  EXPECT_TRUE(port == 80);
}

// non-existent file name
TEST_F(NginxConfigParserTestFixture, NonexistentFilename) {
  EXPECT_FALSE(parser.Parse("blah", &out_config));
}

// nested loop + run throug ToString
TEST_F(NginxConfigParserTestFixture, NestLoopConfig) {
  bool success = parser.Parse("./config_parser_tests/nested_loop", &out_config);
  std::string config_string = out_config.ToString();
  EXPECT_TRUE(success);
}

// comment token
TEST_F(NginxConfigParserTestFixture, CommentToken) {
  EXPECT_TRUE(parser.Parse("./config_parser_tests/comment_token", &out_config));
}

// double quote token
TEST_F(NginxConfigParserTestFixture, DoubleQuote) {
  bool success =
      parser.Parse("./config_parser_tests/double_quote", &out_config);
  EXPECT_TRUE(success);
}

// single quote token
TEST_F(NginxConfigParserTestFixture, SingleQuote) {
  bool success =
      parser.Parse("./config_parser_tests/single_quote", &out_config);
  EXPECT_TRUE(success);
}

// invalid statement ending
TEST_F(NginxConfigParserTestFixture, StatementEndError) {
  bool success =
      parser.Parse("./config_parser_tests/invalid_statement_end", &out_config);
  EXPECT_FALSE(success);
}

// invalid eof
TEST_F(NginxConfigParserTestFixture, InvalidEOF) {
  bool success = parser.Parse("./config_parser_tests/invalid_eof", &out_config);
  EXPECT_FALSE(success);
}

// invalid token type
TEST_F(NginxConfigParserTestFixture, InvalidToken) {
  bool success = ParseString(";***");
  std::string config_string = out_config.ToString();
  EXPECT_FALSE(success);
}

// invalid block start
TEST_F(NginxConfigParserTestFixture, InvalidBlockStart) {
  EXPECT_FALSE(
      parser.Parse("./config_parser_tests/invalid_block_start", &out_config));
}

TEST_F(NginxConfigParserTestFixture, DoubleEndl) {
  EXPECT_FALSE(parser.Parse("./config_parser_tests/double_endl", &out_config));
}

TEST_F(NginxConfigParserTestFixture, SingleEndl) {
  EXPECT_FALSE(parser.Parse("./config_parser_tests/single_endl", &out_config));
}

TEST_F(NginxConfigParserTestFixture, InvalidLeftBlock) {
  EXPECT_FALSE(
      parser.Parse("./config_parser_tests/invalid_left_block", &out_config));
}

TEST_F(NginxConfigParserTestFixture, CredentialsTest) {
  // Example config string
  std::string config = R"(
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

  // Parse the config string
  ASSERT_TRUE(ParseString(config));

  // Call get_credentials method
  std::map<std::string, std::string> credentials = out_config.get_credentials();

  // Define the expected credentials
  std::map<std::string, std::string> expected_credentials = {
      {"tariq", "123"}, {"milly", "456"}, {"shravan", "789"}, {"umer", "101"}};

  // Assert that the credentials match the expected values
  EXPECT_EQ(credentials, expected_credentials);
}

TEST_F(NginxConfigParserTestFixture, AuthTimeTest) {
  // Example config string
  std::string config = R"(
  server {
      port 80;
      timer 20;
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

  // Parse the config string
  ASSERT_TRUE(ParseString(config));

  // Call get_auth_time method
  short auth_time = out_config.get_auth_time();

  // Define the expected auth_time
  short expected_auth_time = 20;

  // Assert that the auth_time matches the expected value
  EXPECT_EQ(auth_time, expected_auth_time);
}