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

	bool is_threated = false; // ���ڹ޾Ҵ���
	bool is_used_skill = false; // ���� ���� ��ų�� ����ߴ��� (��ȸ�� ������)
	bool is_tempted = false; // ��Ȥ ���ߴ���
	bool is_voted = false;  // ��ǥ �ߴ���
	bool is_yesno_voted = false; // ���� ��ǥ �ߴ���
	bool is_mafia_connected = false;
	bool is_final_objection = false; // ���� �ڽ��� ������ �ݷ� ������
	bool is_modified_time = false; // ���� / �ô��� �ߴ���
	int voted_cnt = 0; // �󸶳� ��ǥ�޾Ҵ���
	int visible_voted_cnt = 0; // �󸶳� ��ǥ�޾Ҵ��� (ȭ�鿡 ���̴� ��)
	// ��ġ���� ���̴°� 1ǥ ���� 2ǥ�� ��ǥ�ż� �̷��� �ʿ���

	int vote_right = 1; // ��ǥ��

	void initialize();

	// �縳Ž�� ������ ���� ���� ����
	// callback ���� �ϴ°� ����� set_hand �Ҷ� ȣ���� �� ����.
	void set_hand(vector<Player>::iterator h);
	vector<Player>::iterator get_hand();

	// �̻�� ���� �ٲ�� ȣ��Ǵ� �ݹ� ����
	// �ݹ��� �����ɶ��� �ѹ� ȣ���!
	void set_callback(function<void(vector<Player>::iterator)> f);

	bool is_connected_assist(); // ������ �������� ���� ����

	// �ڽ��� ���� ����� ����
	int team();
};

