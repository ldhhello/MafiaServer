#include "Room.h"
#include "Strings.h"
#include "Method.h"
#include "Utils.h"
#include "MessageQueue.h"
#include <boost/format.hpp>
#include "ChatFilter.h"

// صباح الخير

extern ChatFilter filter;

priority_queue<int, vector<int>, greater<int>> room_number_queue;
map<int, Room*> room_list;
int context_no = 0;
recursive_mutex room_mutex;

vector<Player>::iterator Room::find_player(socket_type key)
{
	for (auto it = player.begin(); it != player.end(); it++)
	{
		if (!it->is_empty && it->key == key)
			return it;
	}

	return player.end();
}

int Room::find_smallest_idx(bool is_empty)
{
	for (int i = 0; i < max_people; i++)
	{
		if (player[i].is_empty == is_empty)
			return i;
	}

	return -1;
}

bool Room::is_valid_index(int idx)
{
    return idx >= 0 && idx < max_people;
}

void Room::broadcast(Packet& packet)
{
	for (auto& p : player)
	{
		if (p.is_empty)
			continue;

		auto& s = p.session;
		s->write_packet(packet);
	}
}

void Room::send_player_list()
{
	Packet packet;
	packet.method = METHOD_PLAYER_LIST;

	for (int i = 0; i < player.size(); i++)
	{
		auto& p = player[i];

		if (p.is_empty)
		{
			packet.push(PLAYER_EMPTY);
			packet.push("");
			continue;
		}

		if (moderator == i)
			packet.push(PLAYER_MODERATOR);
		else
			packet.push(PLAYER_NORMAL);

		packet.push(p.nickname);
	}

	broadcast(packet);
}

Room::Room(asio::io_context& context_, const string& name, int max,
	bool is_password_room, const string& password) :
	context(context_),
	timer(context_),
	room_name(name),
	max_people(max),
	is_password_room(is_password_room),
	password(password)
{
	lock_guard<recursive_mutex> guard(room_mutex);

	room_number = room_number_queue.top();
	room_number_queue.pop();

	if (room_list[room_number] != NULL) // 그럴일은 없겠지만
		delete room_list[room_number];

	room_list[room_number] = this;

	player.resize(max_people);

	now_people = 0;
}

Room::~Room()
{
	//cout << "Room Deleted!" << endl;
}

void Room::delete_this()
{
	lock_guard<recursive_mutex> guard(room_mutex);

	if (room_list[room_number] != this) // 그럴일은 없겠지만
	{
		cout << "Room::delete_this: Something went wrong!" << endl;
		return;
	}
	
	room_list.erase(room_number);
	room_number_queue.push(room_number);
	delete this;
}

void Room::join_room(shared_ptr<Session> session)
{
	asio::post(context, [this, session]
	{
		if (now_people == max_people)
			return;

		if (game != NULL && game->check_if_started())
			return;

		Packet packet;
		packet.method = METHOD_PLAYER_JOINED;
		packet.push(session->nickname);

		broadcast(packet);

		socket_type key = session->session_no;
		//player[key].session = session;

		int idx = find_smallest_idx();
		player[idx].is_empty = false;
		player[idx].key = key;
		player[idx].session = session;
		player[idx].nickname = session->nickname;

		now_people++;

		send_player_list();
	});
}

void Room::leave_room(shared_ptr<Session> session)
{
	asio::post(context, [this, session]
	{
		debug_log << "Session is leaving room..." << endl;
		
		socket_type key = session->session_no;

		auto it = find_player(key);

		if (it == player.end())
		{
			return;
		}

		it->session.reset();
		it->key = 0;
		it->nickname = 
			(boost::format("플레이어 %d") % (distance(player.begin(), it) + 1)).str();
		it->is_empty = true;

		now_people--;

		Packet packet;
		packet.method = METHOD_PLAYER_LEFT;
		packet.push(session->nickname);

		broadcast(packet);

		if (player[moderator].is_empty) // 방장이 떠났음!!
		{
			moderator = find_smallest_idx(false); // 비어있지 않은 가장 빠른 플레이어가 방장이 됨.

			if (moderator != -1)
			{
				Packet packet;
				packet.method = METHOD_MODERATOR_CHANGED;
				packet.push(player[moderator].session->nickname);

				broadcast(packet);
			}
		}

		send_player_list();

		if (now_people == 0)
		{
			delete_this();
		}

		debug_log << "Session left room!" << endl;
		return;
	});
}

void Room::observer_join_room(shared_ptr<Session> session)
{

}

void Room::observer_leave_room(shared_ptr<Session> session)
{

}

void Room::increase_time(shared_ptr<Session> session)
{
	asio::post(context, [this, session]
	{
		if (game == NULL || !game->check_if_started())
			return;

		socket_type key = session->session_no;

		auto it = find_player(key);

		if (it == player.end())
			return;

		game->increase_time(it);
	});
}

void Room::decrease_time(shared_ptr<Session> session)
{
	asio::post(context, [this, session]
	{
		if (game == NULL || !game->check_if_started())
			return;

		socket_type key = session->session_no;

		auto it = find_player(key);

		if (it == player.end())
			return;

		game->decrease_time(it);
	});
}

void Room::chat(shared_ptr<Session> session, const string& str)
{
	asio::post(context, [this, session, str]
	{
		socket_type key = session->session_no;

		auto it = find_player(key);

		if (it == player.end())
			return;

		string filtered_str = str;
		filter.filter(filtered_str);

		if (game != NULL && game->check_if_started())
		{
			game->forward(it, filtered_str);
			return;
		}

		Packet packet;
		packet.method = METHOD_CHAT;

		packet.push(CHAT_NORMAL);
        //packet.push(CHAT_CULT);
		packet.push(session->nickname);
		packet.push(filtered_str);

		for (auto& p : player)
		{
			if (p.is_empty)
				continue;

			p.session->write_packet(packet);
		}
	});
}

void Room::start_game()
{
	asio::post(context, [this]()
	{
		if (game != NULL && game->check_if_started())
			return;

		Packet packet;
		packet.method = METHOD_START_GAME;

		broadcast(packet);

		// 여기에 게임 시작 코드 추가할꺼임
		game = make_shared<Game>(player, now_people, context);

		game->start();
	});
}

void Room::vote(shared_ptr<Session> session, int idx)
{
    if(!is_valid_index(idx))
        return;
    
	asio::post(context, [this, session, idx]()
	{
		auto it = find_player(session->session_no);

		if (it == player.end())
			return;

		if (game == NULL || !game->check_if_started())
			return;

		if (game->vote(it, next(player.begin(), idx)))
		{
			Packet packet;
			packet.method = METHOD_VOTE;
			session->write_packet(packet);
		}
	});
}

void Room::yesno_vote(shared_ptr<Session> session, bool is_yes)
{
	asio::post(context, [this, session, is_yes]()
	{
		auto it = find_player(session->session_no);

		if (it == player.end())
			return;

		if (game == NULL || !game->check_if_started())
			return;

		game->yesno_vote(it, is_yes);
	});
}

void Room::hand(shared_ptr<Session> session, int idx)
{
    if(!is_valid_index(idx))
        return;
    
	asio::post(context, [this, session, idx]()
	{
		auto it = find_player(session->session_no);
		auto target = next(player.begin(), idx);

		if (it == player.end())
			return;

		if (target->is_empty)
			return;

		if (game == NULL || !game->check_if_started())
			return;

		if (game->hand(it, next(player.begin(), idx)))
		{
			Packet packet;
			packet.method = METHOD_HAND;
			session->write_packet(packet);
		}
	});
}

