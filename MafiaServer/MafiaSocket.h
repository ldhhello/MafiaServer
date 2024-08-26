#pragma once
#include "Include.h"
#include "Packet.h"

class MafiaSocket
{
private:
	asio::io_context& context;
	shared_ptr<Socket> sock;
	asio::deadline_timer deadline;
	int packet_len;
	string packet_str;
	recursive_mutex m;
	string buffer; // 이거 recursive_mutex 걸고 써야됨

public:
	shared_ptr<MafiaSocket> this_shared;

	MafiaSocket(asio::io_context& c, shared_ptr<Socket> s);
	~MafiaSocket();

	void close(bool is_force = false);
private:
	bool write(const void* buf, int size);

public:
	bool write_packet(const Packet& packet);
	void async_read_packet(Packet& packet, function<void(bool, Packet&)> callback);

private:
	void on_read_packet(Packet& packet, function<void(bool, Packet&)> callback, const boost::system::error_code& ec, size_t read_sz);
	void check_deadline(const boost::system::error_code& error);
};