#ifndef LOGGER_H
#define LOGGER_H

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
#include <string>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace logging_trivial = boost::log::trivial;

class Logger {
public:
  static Logger *getLogger() {
    if (logger == nullptr) {
      logger = new Logger();
    }
    return logger;
  }

  void init();
  void logServerInitialization();
  void logTraceFile(const std::string &trace_message);
  void logErrorFile(const std::string &error_message);
  void logDebugFile(const std::string &debug_message);
  void logWarningFile(const std::string &warning_message);
  void logSig();
  void logTrace();
  void logDebug();
  void logWarning();
  void logError();
  void logFatal();
  void logTraceHTTPrequest(const boost::beast::http::request<boost::beast::http::string_body> &http_request, boost::asio::ip::tcp::socket &m_socket);
  void logResponse(const std::string &response_message);
  Logger();
  static Logger *logger;

private:
  src::severity_logger<logging_trivial::severity_level> lg;
};

#endif // LOGGER_H
