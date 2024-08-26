#include "Session.h"
#include "Utils.h"
#include "Method.h"
#include "Room.h"
#include "Strings.h"
#include "ChatFilter.h"
#include <set>
#include <ctime>

extern ChatFilter filter;

// 여기 있는 닉네임은 못씀, 없는 닉네임은 쓸수있음
recursive_mutex nickname_mutex;
set<string> nickname_list;

Session::Session(asio::io_context& context, shared_ptr<Socket> stream) :
	sock(new MafiaSocket(context, stream)),
	context(context)
{
	sock->this_shared = sock;
	session_no = stream->lowest_layer().native_handle();

	debug_log << "Session connected!" << endl;
}

Session::~Session()
{
	{
		lock_guard<recursive_mutex> guard(nickname_mutex);
		if (nickname_list.find(nickname) != nickname_list.end())
		{
			nickname_list.erase(nickname);
			return;
		}
	}

	debug_log << "Session deleted!" << endl;
}

void Session::run()
{
	init();

	try
	{
		handle_client();
	}
	catch (...) {}
}

void Session::close()
{
	debug_log << "Session : closing..." << endl;

	if (room_no != -1)
	{
		lock_guard<recursive_mutex> guard(room_mutex);

		if (room_list.find(room_no) != room_list.end())
		{
			room_list[room_no]->leave_room(this_shared);
		}
	}
    
    is_closed = true;

	sock.reset();
	this_shared.reset();
}

void Session::init()
{
}

void Session::handle_client()
{
	packet.clear();
	packet.method = METHOD_HELLO;
	// ex) Mafia_Server 1.0_Beta
	packet.push(SERVER_NAME " " MAFIA_PROTOCOL_VERSION);

	sock->write_packet(packet);

	sock->async_read_packet(recv_packet, bind(&Session::on_read_packet, shared_from_this(), placeholders::_1));
}

void Session::on_read_packet(bool is_success)
{
	if (!is_success)
	{
		//cout << "Error on Session::on_read_packet" << endl;
		close();
		return;
	}

	if (!is_nickname_initialized &&
		recv_packet.method != METHOD_PING &&
		recv_packet.method != METHOD_NICKNAME)
	{
		cout << "ERROR: Other method was called before METHOD_NICKNAME" << endl;
		close();
		return;
	}

    time_t current_time = time(NULL);
    
    if(current_time != last_packet_time)
    {
        last_packet_time = current_time;
        packet_cnt = 0;
    }
    
    packet_cnt++;
    
    if(packet_cnt > 60)
    {
        cout << "ERROR: Too many requests" << endl;
        close();
        return;
    }
    
	switch (recv_packet.method)
	{
		HANDLE_METHOD(METHOD_PING, method_ping, 0);
		HANDLE_METHOD(METHOD_NICKNAME, method_nickname, 1);
		HANDLE_METHOD(METHOD_ROOM_LIST, method_room_list, 0);
		HANDLE_METHOD(METHOD_CREATE_ROOM, method_create_room, 4);
		HANDLE_METHOD(METHOD_JOIN_ROOM, method_join_room, 1);
		HANDLE_METHOD(METHOD_LEAVE_ROOM, method_leave_room, 0);
		HANDLE_METHOD(METHOD_JOIN_PASSWORD_ROOM, method_join_password_room, 2);
		HANDLE_METHOD(METHOD_CHAT, method_chat, 1);
		HANDLE_METHOD(METHOD_START_GAME, method_start_game, 0);
		HANDLE_METHOD(METHOD_INDEX, method_index, 0);
		HANDLE_METHOD(METHOD_VOTE, method_vote, 1);
		HANDLE_METHOD(METHOD_YESNO_VOTE, method_yesno_vote, 1);
		HANDLE_METHOD(METHOD_HAND, method_hand, 1);
		HANDLE_METHOD(METHOD_INCREASE_TIME, method_increase_time, 0);
		HANDLE_METHOD(METHOD_DECREASE_TIME, method_decrease_time, 0);

	default:
		close();
		return;
	}
    
    if(is_closed)
        return;

	sock->async_read_packet(recv_packet, bind(&Session::on_read_packet, shared_from_this(), placeholders::_1));
}

void Session::method_ping()
{
	packet.clear();
	packet.method = METHOD_PONG;

	sock->write_packet(packet);
}

// 인수 0: 플레이어의 닉네임
void Session::method_nickname()
{
    if(is_nickname_initialized)
    {
        debug_log << "Nickname re-initialized!" << endl;
        close();
        return;
    }
    
	debug_log << "Nickname: " << recv_packet.get_string(0) << endl;
	nickname = recv_packet.get_string(0);

	if (filter.has_dirty_word(nickname))
	{
		debug_log << "Nickname \"" << nickname << "\" has dirty word" << endl;

		packet.clear();
		packet.method = METHOD_ERROR;
		packet.push(STRING_DIRTY_WORD_NICKNAME);
		sock->write_packet(packet);

		nickname = "";
		close();
        
		return;
	}

	if (!Utils::is_valid_nickname(nickname))
	{
		debug_log << "Nickname \"" << nickname << "\" is not valid" << endl;

		packet.clear();
		packet.method = METHOD_ERROR;
		packet.push(STRING_INVALID_NICKNAME);
		sock->write_packet(packet);

		nickname = "";
		close();
		return;
	}

	{
		lock_guard<recursive_mutex> guard(nickname_mutex);
		if (nickname_list.find(nickname) != nickname_list.end())
		{
			debug_log << "Nickname \"" << nickname << "\" already exists" << endl;

			packet.clear();
			packet.method = METHOD_ERROR;
			packet.push(STRING_NICKNAME_ALREADY_EXISTS);

			sock->write_packet(packet);

			nickname = "";

			close();
			return;
		}

        cout << "New nickname: " << nickname << endl;
		nickname_list.insert(nickname);
		is_nickname_initialized = true;
	}
}

void Session::method_room_list()
{
    if(room_no != -1)
        return;
    
	packet.clear();
	packet.method = METHOD_ROOM_LIST;

	{
		lock_guard<recursive_mutex> guard(room_mutex);
		for (auto& p : room_list)
		{
			Room* room = p.second;

			if (room->game != NULL && room->game->check_if_started())
				continue;

			packet.push(room->get_room_number());
			packet.push(room->get_room_name());
			packet.push(room->get_now_people());
			packet.push(room->get_max_people());
			packet.push(room->is_password_room);
		}
	}

	//cout << "packet size: " << packet.vec.size() << endl;
	sock->write_packet(packet);
}

void Session::method_create_room()
{
	string room_name =        recv_packet.get_string(0);
	int max_people =          recv_packet.get_int(1);
	bool is_password_room =   recv_packet.get_int(2);
	const string& password =  recv_packet.get_string(3);
    
    if(room_no != -1)
        return;

    // 12인 지원 추가
	if (max_people < 4 || max_people > 12)
	{
		packet.clear();
		packet.method = METHOD_ERROR;
		packet.push(STRING_ROOM_SIZE_IS_INVALID);

		sock->write_packet(packet);
		return;
	}

	int c_no;
	{
		lock_guard<recursive_mutex> guard(room_mutex);
		c_no = context_no;
		context_no++;
		if (context_no >= THREAD_COUNT)
			context_no = 0;
	}
	
	filter.filter(room_name);

	Room* room = new Room(::context[c_no], room_name, max_people,
		is_password_room, password);

	room->join_room(this_shared);
	room_no = room->get_room_number();
	entered_room = room;

	packet.clear();
	packet.method = METHOD_JOIN_ROOM;
	packet.push(room->get_room_number());
	packet.push(room->get_room_name());
	packet.push(room->get_now_people());
	packet.push(room->get_max_people());

	sock->write_packet(packet);
}

void Session::method_join_room()
{
	if (room_no != -1)
		return;

	int room_no_ = recv_packet.get_int(0);

	Room* room = NULL;

	{
		lock_guard<recursive_mutex> guard(room_mutex);

		if (room_list.find(room_no_) != room_list.end())
		{
			if (room_list[room_no_]->is_password_room)
				return;

			if (room_list[room_no_]->get_now_people() == room_list[room_no_]->get_max_people())
			{
				packet.clear();
				packet.method = METHOD_ERROR;
				packet.push(STRING_ROOM_IS_FULL);

				sock->write_packet(packet);
				return;
			}

			room = room_list[room_no_];
		}
		else
		{
			packet.clear();
			packet.method = METHOD_ERROR;
			packet.push(STRING_ROOM_DOES_NOT_EXIST);

			sock->write_packet(packet);
			return;
		}
        
        room_no = room_no_;
        entered_room = room;

        room->join_room(this_shared);
	}

	packet.clear();
	packet.method = METHOD_JOIN_ROOM;
	packet.push(room->get_room_number());
	packet.push(room->get_room_name());
	packet.push(room->get_now_people());
	packet.push(room->get_max_people());

	sock->write_packet(packet);
}

void Session::method_leave_room()
{
	if (room_no == -1)
		return;

	entered_room->leave_room(this_shared);

	room_no = -1;
	entered_room = nullptr;

	packet.clear();
	packet.method = METHOD_LEAVE_ROOM;
	sock->write_packet(packet);
}

void Session::method_join_password_room()
{
	if (room_no != -1)
		return;

	int room_no_ = recv_packet.get_int(0);
	const string& password = recv_packet.get_string(1);

	Room* room = NULL;

	{
		lock_guard<recursive_mutex> guard(room_mutex);

		if (room_list.find(room_no_) != room_list.end())
		{
			if (!room_list[room_no_]->is_password_room)
				return;
			if (room_list[room_no_]->password != password)
			{
				packet.clear();
				packet.method = METHOD_ERROR;
				packet.push(STRING_INVALID_PASSWORD);

				sock->write_packet(packet);
				return;
			}

			if (room_list[room_no_]->get_now_people() == room_list[room_no_]->get_max_people())
			{
				packet.clear();
				packet.method = METHOD_ERROR;
				packet.push(STRING_ROOM_IS_FULL);

				sock->write_packet(packet);
				return;
			}

			room = room_list[room_no_];
		}
		else
		{
			packet.clear();
			packet.method = METHOD_ERROR;
			packet.push(STRING_ROOM_DOES_NOT_EXIST);

			sock->write_packet(packet);
			return;
		}
        
        room_no = room_no_;
        entered_room = room;

        room->join_room(this_shared);
	}

	packet.clear();
	packet.method = METHOD_JOIN_ROOM;
	packet.push(room->get_room_number());
	packet.push(room->get_room_name());
	packet.push(room->get_now_people());
	packet.push(room->get_max_people());

	sock->write_packet(packet);
}

// 단시간에 너무 많은 채팅이 오면 무시해버린다!
void Session::method_chat()
{
    time_t current_time = time(NULL);
    
    if(current_time != last_chat_time)
    {
        last_chat_time = current_time;
        chat_cnt = 0;
    }
    
    chat_cnt++;
    
    if(chat_cnt > 5)
        return;
    
	if (room_no == -1)
		return;

	const string& str = recv_packet.get_string(0);

	entered_room->chat(this_shared, str);
}

void Session::method_start_game()
{
	if (room_no == -1)
		return;

	if (!entered_room->is_moderator(this_shared))
		return; // 이 경우도 일반적인 클라이언트면 있을리가 없음

	if (entered_room->get_now_people() < 4)
	{
		packet.clear();
		packet.method = METHOD_ERROR;
		packet.push(STRING_PLAYER_NOT_ENOUGH);

		sock->write_packet(packet);
		return;
	}

	entered_room->start_game();
}

void Session::method_index()
{
	if (room_no == -1)
		return;

	packet.clear();
	packet.method = METHOD_INDEX;
	int idx = distance(
		entered_room->player.begin(), 
		entered_room->find_player(session_no)
	);

	packet.push(idx);

	sock->write_packet(packet);
}

void Session::method_vote()
{
	if (room_no == -1)
		return;

	int idx = recv_packet.get_int(0);
	entered_room->vote(this_shared, idx);
}

void Session::method_yesno_vote()
{
	if (room_no == -1)
		return;

	int is_yes = recv_packet.get_int(0);
	entered_room->yesno_vote(this_shared, is_yes);
}

void Session::method_hand()
{
	if (room_no == -1)
		return;

	int idx = recv_packet.get_int(0);
	entered_room->hand(this_shared, idx);
}

void Session::method_increase_time()
{
	if (room_no == -1)
		return;

	entered_room->increase_time(this_shared);
}

void Session::method_decrease_time()
{
	if (room_no == -1)
		return;

	entered_room->decrease_time(this_shared);
}
