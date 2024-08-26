#pragma once
#include "Session.h"
#include "Job.h"

enum PlayerStatus
{
	PLAYER_ALIVE,
	PLAYER_DEAD,
	PLAYER_SEONGBUL
};

class Player
{
public:
	bool is_empty = true;
	socket_type key;
	shared_ptr<Session> session;
	Job job;
	int status = PLAYER_ALIVE;
	vector<Player>::iterator vote_hand;

private:
	vector<Player>::iterator hand;
	function<void(vector<Player>::iterator)> callback;

public:
	string nickname;

	bool is_threated = false; // 협박받았는지
	bool is_used_skill = false; // 게임 동안 스킬을 사용했는지 (일회용 직업만)
	bool is_tempted = false; // 유혹 당했는지
	bool is_voted = false;  // 투표 했는지
	bool is_yesno_voted = false; // 찬반 투표 했는지
	bool is_mafia_connected = false;
	bool is_final_objection = false; // 현재 자신의 최후의 반론 중인지
	bool is_modified_time = false; // 시증 / 시단을 했는지
	int voted_cnt = 0; // 얼마나 투표받았는지
	int visible_voted_cnt = 0; // 얼마나 투표받았는지 (화면에 보이는 것)
	// 정치인이 보이는거 1표 실제 2표로 투표돼서 이런게 필요함

	int vote_right = 1; // 투표권

	void initialize();

	// 사립탐정 지원을 위해 만든 물건
	// callback 세팅 하는걸 만들고 set_hand 할때 호출해 줄 거임.
	void set_hand(vector<Player>::iterator h);
	vector<Player>::iterator get_hand();

	// 이사람 손이 바뀌면 호출되는 콜백 지정
	// 콜백이 지정될때도 한번 호출됨!
	void set_callback(function<void(vector<Player>::iterator)> f);

	bool is_connected_assist(); // 접선한 보조인지 여부 리턴

	// 자신의 팀이 어딘지 리턴
	int team();
};

