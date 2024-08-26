#pragma once
#include "Include.h"

class Server
{
private:
	asio::io_context m_context;

	void run_context(asio::io_context& c);
	void run_thread(asio::io_context& c);

public:
	void init();
	void run(Port port);
};

