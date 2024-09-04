#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/bind.hpp> // Add this include for Bind placeholders
#include <cstdlib>
#include <iostream>

#include "logger.h"
#include "server.h"
#include "session.h"
using boost::asio::ip::tcp;

server::server(boost::asio::io_service &io_service, short port,
               const NginxConfig &config,
               const std::map<std::string, std::string> &credentials,
               short auth_time)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      dispatcher_(std::make_shared<RequestHandlerDispatcher>(config)),
      credentials_(credentials), auth_time_(auth_time) {
        std::cout << "helgsdgs: " << auth_time_;
  auto m_session =
      std::make_shared<session>(io_service_, dispatcher_, credentials_, auth_time_);
  start_accept(*m_session);
}

void server::start_accept(session &m_session) {
  auto new_session =
      std::make_shared<session>(io_service_, dispatcher_, credentials_, auth_time_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                                     boost::asio::placeholders::error));
}

void server::handle_accept(std::shared_ptr<session> new_session,
                           const boost::system::error_code &error) {
  if (!error)
    new_session->start();

  start_accept(*new_session);
}
