#pragma once
#include "Include.h"

class Acceptor
{
private:
	asio::io_context& m_ios;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ssl::context m_ssl_context;
	int i = 0;

public:
	Acceptor(asio::io_context& ios, Port port_num);
	void on_accept(shared_ptr<Socket> sock);
	void on_handshaked(shared_ptr<Socket> sock, int idx, const boost::system::error_code& e);
};
