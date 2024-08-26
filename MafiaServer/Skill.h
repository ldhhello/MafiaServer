#pragma once
#include "Include.h"

class Game;
class Player;
class Packet;

class Skill
{
private:
	Game* game;
	vector<Player>::iterator mafia_gun_muzzle;

	void broadcast(Packet& p);

public:
	Skill(Game* g);

	// 능력을 사용함. 능력 사용 성공 -> true
	bool use(vector<Player>::iterator p, vector<Player>::iterator target);

	// 투표할 때 능력을 사용함. (마담, 도둑 등)
	void vote_use(vector<Player>::iterator p, vector<Player>::iterator target);

	// 시간이 바낄 때 Game에서 호출해 줌.
	void on_time_changed(int t);

private:
	// 손이 움직이는 직업이 특정 플레이어에게 손을 옮길 수 있는지 여부 리턴
	bool can_change_hand
	(vector<Player>::iterator p, vector<Player>::iterator target);
	bool use_fixed_hand
	(vector<Player>::iterator p, vector<Player>::iterator target);

	// 보조 p가 마피아와 접선함
	void connect(vector<Player>::iterator p);
	void mafia_kill();
	void ghoul(); // 도굴꾼이 있다면 도굴꾼이 직업을 도굴해 감
	void send_status(vector<Player>::iterator p); // p 현재 상태 모두에게 전달
	void on_day_started();

	// target이 지금 의사의 치료를 받는지 여부 리턴
	bool is_doctor_heal(vector<Player>::iterator target);

	// 짐승인간의 손을 리턴. 없으면 player.end()
	vector<Player>::iterator get_beastman_hand();

public:
	void vote_kill(vector<Player>::iterator p); // p는 투표로 죽는 사람

private:
	void on_other_hand(
		vector<Player>::iterator detective, 
		vector<Player>::iterator hand
	);
};

#include "Game.h"