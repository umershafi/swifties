#include "config_parser.h"
#include "logger.h"
#include "server.h"
#include "session.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <csignal>
#include <cstdlib>
#include <iostream>

#include "config_parser.h"
#include "server.h"
#include "session.h"

using boost::asio::ip::tcp;

void signalHandler(int signal) {
  Logger *logger = Logger::getLogger();
  logger->logSig();
  exit(1); // Exit program
}

int main(int argc, char *argv[]) {
  NginxConfigParser parser;
  NginxConfig config;
  Logger *logger = Logger::getLogger();
  try {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    if (argc != 2) {
      logger->logErrorFile("Wrong usage port is needed");
      return 1;
    }

    logger->logTraceFile("Parsing config file ...");
    if (!parser.Parse(argv[1], &config)) {
      logger->logErrorFile("Unable to parse config file");
      return -1;
    }
    logger->logTraceFile("Parsed!");

    int port;
    logger->logTraceFile("Checking port ...");
    if ((port = config.get_config_port()) == -1) {
      logger->logErrorFile("Invalid Port");
      return -1;
    }
    logger->logTraceFile("Port Retrieved!");

    short auth_time;
    logger->logTraceFile("Checking auth time ...");
    if ((auth_time = config.get_auth_time()) == -1) {
      logger->logErrorFile("Invalid Auth Time");
      return -1;
    }
    logger->logTraceFile("Auth Time Retrieved!");

    // Store credentials from the config
    std::map<std::string, std::string> credentials = config.get_credentials();

    boost::asio::io_service io_service;
    server s(io_service, static_cast<short>(port), config, credentials, auth_time);
    logger->logServerInitialization();
    logger->logTraceFile("Starting server on port " + std::to_string(port));
    logger->logTraceFile("Authorization timeout: " + std::to_string(auth_time) + " seconds");

    io_service.run();
  } catch (std::exception &e) {
    logger->logErrorFile(std::string("Exception: ") + e.what());
  }

  return 0;
}
