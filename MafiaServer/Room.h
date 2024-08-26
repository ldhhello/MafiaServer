#pragma once
#include "Include.h"
#include "Player.h"
#include "Session.h"

class Game;

class Room
{
public:
	friend class Session;

	enum ChatType
	{
		CHAT_NULL = 0,
		CHAT_NORMAL = 1,
		CHAT_DEAD = 2,
		CHAT_MAFIA = 3,
		CHAT_COUPLE = 4,
		CHAT_SYSTEM = 5,
		CHAT_CULT = 6
	};

	enum PlayerStatus
	{
		PLAYER_EMPTY = 0,
		PLAYER_NORMAL = 1,
		PLAYER_MODERATOR = 2
	};

private:
	int room_number;
	string room_name;
	atomic<int> now_people;
	int max_people;
	bool is_password_room;
	string password;

	int moderator = 0; // 방장 픽 번호

	shared_ptr<Game> game;

	asio::io_context& context;
	asio::deadline_timer timer;

	vector<Player> player;
	vector<Player> observer;

	vector<Player>::iterator find_player(socket_type key);
	int find_smallest_idx(bool is_empty = true); // player 벡터에서 사용 가능한 가장 작은 픽번호 리턴
    
    bool is_valid_index(int idx); // 유효한 픽번호인지 (-1번, 1000번, ...)

private:
	void broadcast(Packet& packet);
	void send_player_list();

public:
	Room(asio::io_context& context_, const string& name, int max,
		bool is_password_room, const string& password);
	~Room();

	void delete_this();

	void join_room(shared_ptr<Session> session);
	void leave_room(shared_ptr<Session> session);

	void observer_join_room(shared_ptr<Session> session);
	void observer_leave_room(shared_ptr<Session> session);

	void increase_time(shared_ptr<Session> session);
	void decrease_time(shared_ptr<Session> session);

	void chat(shared_ptr<Session> session, const string& str);
	void start_game();
	void vote(shared_ptr<Session> session, int idx);
	void yesno_vote(shared_ptr<Session> session, bool is_yes);
	void hand(shared_ptr<Session> session, int idx);

public:
	bool is_moderator(shared_ptr<Session> session) { return distance(player.begin(), find_player(session->session_no)) == moderator; }

	int get_room_number() { return room_number; }
	string& get_room_name() { return room_name; }
	int get_now_people() { return now_people; }
	int get_max_people() { return max_people; }
};

extern priority_queue<int, vector<int>, greater<int>> room_number_queue;
extern map<int, Room*> room_list;
extern int context_no;
extern recursive_mutex room_mutex;

#include "Game.h"
