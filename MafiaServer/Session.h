#pragma once

#include "Include.h"
#include "MafiaSocket.h"

class Room;

class Session :
	public enable_shared_from_this<Session>
{
private:
	shared_ptr<MafiaSocket> sock;
	Packet recv_packet;
	Packet packet;
	asio::io_context& context;
	int room_no = -1;
	bool is_nickname_initialized = false;
	Room* entered_room = nullptr;
    bool is_closed = false;
    
    time_t last_packet_time = -1;
    int packet_cnt = 0;
    
    time_t last_chat_time = -1;
    int chat_cnt = 0;

public:
	string nickname;

public:
	void write_packet(const Packet& packet) 
	{ 
		if (this == NULL)
			return;

		asio::post(context, [this, packet] { sock->write_packet(packet); }); 
	}

public:
	shared_ptr<Session> this_shared;
	socket_type session_no;

public:
	Session(asio::io_context& context, shared_ptr<Socket> stream);
	~Session();
	void run();

private:
	void close();
	void init();
	void handle_client();

private:
	void on_read_packet(bool is_success);

private:
	void method_ping();
	void method_nickname();
	void method_room_list();
	void method_create_room();
	void method_join_room();
	void method_leave_room();
	void method_join_password_room();
	void method_chat();
	void method_start_game();
	void method_index();
	void method_vote();
	void method_yesno_vote();
	void method_hand();
	void method_increase_time();
	void method_decrease_time();
};
