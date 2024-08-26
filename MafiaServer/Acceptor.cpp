#include "Acceptor.h"
#include "Session.h"

#if defined DEBUG
const string cert_file = "/Users/donghyun/MafiaServer_data/cert.pem";
const string key_file = "/Users/donghyun/MafiaServer_data/key.pem";
#else
const string cert_file = "/key/cert.pem";
const string key_file = "/key/key.pem";
#endif

Acceptor::Acceptor(asio::io_context& ios, Port port_num) :
	m_ios(ios),
	m_acceptor(m_ios,
		asio::ip::tcp::endpoint(
			asio::ip::address_v4::any(),
			port_num
		)),
	m_ssl_context(asio::ssl::context::sslv23_server)
{
	m_ssl_context.set_options(
		asio::ssl::context::default_workarounds |
		asio::ssl::context::no_sslv2
	);

	//string& cert_file = reader.tls_cert_file;
	//string& key_file = reader.tls_key_file;

	m_ssl_context.use_certificate_chain_file(cert_file);
	m_ssl_context.use_private_key_file(key_file,
		boost::asio::ssl::context::pem);

	m_acceptor.listen();
	shared_ptr<Socket> sock = make_shared<Socket>(context[0], m_ssl_context);
	m_acceptor.async_accept(sock->lowest_layer(), bind(&Acceptor::on_accept, this, sock));
}

void Acceptor::on_accept(shared_ptr<Socket> sock)
{
	try
	{
        //cout << "Connected: " << sock->lowest_layer().remote_endpoint() << endl;
        
		sock->async_handshake(asio::ssl::stream_base::server,
			bind(&Acceptor::on_handshaked, this, sock, i, placeholders::_1));

		/*auto session = make_shared<Session>(context[i], sock);
		session->this_shared = session;
		session->run();*/
	}
	catch (...) {}

	try
	{
		i++;
		if (i == THREAD_COUNT)
			i = 0;

		shared_ptr<Socket> new_sock = make_shared<Socket>(context[i], m_ssl_context);
		m_acceptor.async_accept(new_sock->lowest_layer(), bind(&Acceptor::on_accept, this, new_sock));
	}
	catch (exception& e)
	{
		cout << "Exception! " << e.what() << endl;
	}
}

void Acceptor::on_handshaked(shared_ptr<Socket> sock, int idx, const boost::system::error_code& e)
{
	if (e)
	{
		cout << "Handshake Error!" << endl;
		return;
	}

	auto session = make_shared<Session>(context[idx], sock);
	session->this_shared = session;
	session->run();
}
