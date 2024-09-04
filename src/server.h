#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include "config_parser.h" 
#include "request_handler_dispatcher.h"
#include "session.h"

using boost::asio::ip::tcp;

class session; // Forward declaration to break circular dependency

class server {
public:
  server(boost::asio::io_service &io_service, short port,
         const NginxConfig &config,
         const std::map<std::string, std::string> &credentials, short auth_time);

  void start_accept(session &m_session);
  void handle_accept(std::shared_ptr<session> new_session,
                     const boost::system::error_code &error);

  boost::asio::io_service &io_service_;
  tcp::acceptor acceptor_;

private:
  std::shared_ptr<RequestHandlerDispatcher> dispatcher_;
  std::map<std::string, std::string> credentials_;
  short auth_time_;
};

#endif // SERVER_H
