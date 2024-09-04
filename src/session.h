#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <chrono>

class RequestHandlerDispatcher;

class session : public std::enable_shared_from_this<session> {
public:
  explicit session(boost::asio::io_service &io_service,
                   std::shared_ptr<const RequestHandlerDispatcher> dispatcher,
                   const std::map<std::string, std::string> &credentials, short auth_time);

  boost::asio::ip::tcp::socket &socket();

  void start();
  int handle_read_callback(std::shared_ptr<session> self,
                           boost::system::error_code error,
                           std::size_t bytes_transferred);

  int handle_write_callback(std::shared_ptr<session> self,
                            boost::system::error_code error,
                            std::size_t bytes_transferred);
  boost::beast::flat_buffer buffer_;

private:
  void handle_read();
  void handle_write();
  bool authenticate(const std::string &auth_header);
  void send_unauthorized_response();
  bool is_session_expired();

  boost::asio::ip::tcp::socket socket_;
  const std::map<std::string, std::string> credentials_;
  std::shared_ptr<const RequestHandlerDispatcher> dispatcher_;
  boost::beast::http::response<boost::beast::http::string_body> response_;
  std::chrono::time_point<std::chrono::steady_clock> last_auth_time_;
  short auth_time_;
};

#endif // SESSION_H
