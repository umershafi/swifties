#include "logger.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/signals2.hpp>
#include <boost/beast/http.hpp>
#include <sstream>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;
using boost::asio::ip::tcp;

Logger::Logger() { init(); }

void Logger::init() {
  logging::add_common_attributes();
  logging::add_file_log(
      keywords::file_name = "../logs/server_log_%Y-%m-%d.log",
      keywords::rotation_size = 10 * 1024 * 1024, // new log at 10mb
      keywords::time_based_rotation =
          boost::log::sinks::file::rotation_at_time_point(
              0, 0, 0), // new log every 24hr
      keywords::format = "[%TimeStamp%]:[%ThreadID%]:%Message%", // line format
      keywords::auto_flush = true);
  logging::add_console_log(std::cout, keywords::format = ">> %Message%");
}
void Logger::logServerInitialization() {
  BOOST_LOG_SEV(lg, trace) << "Server has been initialized";
}
void Logger::logTraceFile(const std::string &trace_message) {
  BOOST_LOG_SEV(lg, trace) << "Trace: " << trace_message;
}
void Logger::logErrorFile(const std::string &error_message) {
  BOOST_LOG_SEV(lg, error) << "Error: " << error_message;
}

void Logger::logDebugFile(const std::string &debug_message) {
  BOOST_LOG_SEV(lg, debug) << "Debug: " << debug_message;
}

void Logger::logResponse(const std::string &response_message) {
  BOOST_LOG_SEV(lg, info) << "ResponseMetrics: " << response_message;
}


void Logger::logWarningFile(const std::string &warning_message) {
  BOOST_LOG_SEV(lg, warning) << "Warning: " << warning_message;
}

void Logger::logSig() {
  BOOST_LOG_SEV(lg, warning) << "Shutting down the server...";
}

void Logger::logTrace() {
  BOOST_LOG_TRIVIAL(trace) << "This is a trace severity message";
}

void Logger::logDebug() {
  BOOST_LOG_TRIVIAL(debug) << "This is a debug severity message";
}

void Logger::logWarning() {
  BOOST_LOG_TRIVIAL(warning) << "This is a warning severity message";
}

void Logger::logError() {
  BOOST_LOG_TRIVIAL(error) << "This is a error severity message";
}

void Logger::logFatal() {
  BOOST_LOG_TRIVIAL(fatal) << "This is a fatal severity message";
}

void Logger::logTraceHTTPrequest(const boost::beast::http::request<boost::beast::http::string_body> &http_request, tcp::socket &m_socket) {
  std::stringstream stream;
  stream << "Trace: ";
  stream << http_request.method_string() << " " << http_request.target() << " HTTP "
         << (http_request.version() / 10) << "." << (http_request.version() % 10);
  // stream << " Sender IP: " << m_socket.remote_endpoint().address().to_string();
  BOOST_LOG_SEV(lg, trace) << stream.str();
}

Logger *Logger::logger = nullptr;
