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

	// �ɷ��� �����. �ɷ� ��� ���� -> true
	bool use(vector<Player>::iterator p, vector<Player>::iterator target);

	// ��ǥ�� �� �ɷ��� �����. (����, ���� ��)
	void vote_use(vector<Player>::iterator p, vector<Player>::iterator target);

	// �ð��� �ٳ� �� Game���� ȣ���� ��.
	void on_time_changed(int t);

private:
	// ���� �����̴� ������ Ư�� �÷��̾�� ���� �ű� �� �ִ��� ���� ����
	bool can_change_hand
	(vector<Player>::iterator p, vector<Player>::iterator target);
	bool use_fixed_hand
	(vector<Player>::iterator p, vector<Player>::iterator target);

	// ���� p�� ���Ǿƿ� ������
	void connect(vector<Player>::iterator p);
	void mafia_kill();
	void ghoul(); // �������� �ִٸ� �������� ������ ������ ��
	void send_status(vector<Player>::iterator p); // p ���� ���� ��ο��� ����
	void on_day_started();

	// target�� ���� �ǻ��� ġ�Ḧ �޴��� ���� ����
	bool is_doctor_heal(vector<Player>::iterator target);

	// �����ΰ��� ���� ����. ������ player.end()
	vector<Player>::iterator get_beastman_hand();

public:
	void vote_kill(vector<Player>::iterator p); // p�� ��ǥ�� �״� ���

private:
	void on_other_hand(
		vector<Player>::iterator detective, 
		vector<Player>::iterator hand
	);
};

#include "Game.h"