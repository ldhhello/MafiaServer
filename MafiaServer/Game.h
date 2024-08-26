#pragma once
#include "Include.h"
#include "Room.h"
#include "Timer.h"

class Skill;

// صباح الخير

enum TimeStatus
{
	TIME_NULL,
	TIME_NIGHT,
	TIME_DAY,
	TIME_VOTE,
	TIME_FINAL_OBJECTION,
	TIME_YESNO_VOTE,
	TIME_END // 마지막
};

enum FinishStatus
{
	NO_FINISH = 0,
	MAFIA_WIN = 1,
	CITIZEN_WIN = 2,
	CULT_WIN = 3
};

// 각 시간의 실제 이름
const string time_str[] =
{
	"",
	"밤",
	"낮",
	"투표 시간",
	"최후의 반론 시간",
	"찬반 투표 시간",
	""
};

// 각 시간이 몇초나 되는지
const int time_length[] =
{
	0,           // NULL 시간 (없는 시간)
	25,          // 밤
	-1,          // 가변 (꺼무위키 "마피아42/게임 방법" 에 따르면 살아있는 유저 수 * 15초)
	15,          // 투표 시간
	15,          // 최후의 반론 시간
	10           // 찬반 투표 시간
	-1
};

class Game
{
private:
	friend class Skill;

	bool is_started = false;
	vector<Player>& player;
	Timer timer;

	vector<Player>::iterator max_player;

	int player_cnt;
	int time_status; // 현재 시간 (밤, 낮, 최후의 반론 등등..)

	bool was_yesno = false;
	int yes_cnt = 0; // 찬성 얼마나 눌렀는지

	shared_ptr<Skill> skill;

	int day_cnt = 0;

public:
	// 두번째 인수는 플레이어 몇명 있는지
	Game(vector<Player>& p, int cnt, asio::io_context& context);

	void broadcast_if(Packet& packet, function<bool(Player&)> condition);
	void broadcast(Packet& packet);
	void forward(vector<Player>::iterator sender, const string& msg);

	// p의 메모장에 target의 직업을 메모해줌.
	void memo(vector<Player>::iterator p, vector<Player>::iterator target);

	// 모든 플레이어에게 target의 직업을 메모해줌.
	void memo_all(vector<Player>::iterator target);

	// vector<int> 속 인덱스들에 해당하는 모든 플레이어들에게 서로의 직업을 메모함
	void memo_each(const vector<int>& list);
    
    // target의 포교 여부를 메모해준다!
    void memo_culted(vector<Player>::iterator p, vector<Player>::iterator target);
    void memo_culted_each(const vector<int>& list);

	// 게임을 시작함
	void start();

private:
	void shuffle_job();
    //void shuffle_job_new(); // 새로운 메커니즘 테스트

	void change_time();
	void on_time_changed();

	void on_time_started();
	void on_time_ended();

	void on_night_started();

	void on_day_started();

	void on_vote_started();
	void on_vote_ended();

	void on_final_objection_started();

	void on_yesno_vote_started();
	void on_yesno_vote_ended();

	int check_finish();

public:
	void increase_time(vector<Player>::iterator p);
	void decrease_time(vector<Player>::iterator p);

	bool check_if_started() { return is_started; }
	bool vote(vector<Player>::iterator p, vector<Player>::iterator target);
	bool yesno_vote(vector<Player>::iterator p, bool is_yes);
	bool hand(vector<Player>::iterator p, vector<Player>::iterator target);
};

#include "Skill.h"
