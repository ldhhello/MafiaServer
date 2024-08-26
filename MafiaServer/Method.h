#pragma once

// ��� ��� ����� Method��.
// ��ȣ �뿪�� �� ����� �ƹ����� ������������

enum Method
{
	METHOD_HELLO = 1,
	METHOD_NICKNAME = 100,
	METHOD_ERROR = 2147483647,
	METHOD_PING = 100000000,
	METHOD_PONG = 100000001,

	// 10000����� ��� ���õ� �޼���
	METHOD_ROOM_LIST = 10000,
	METHOD_CREATE_ROOM = 10001,
	METHOD_JOIN_ROOM = 10002,
	METHOD_LEAVE_ROOM = 10003,
	METHOD_CHAT = 10004,
	METHOD_SYSTEM_CHAT = 10005,
	METHOD_PLAYER_JOINED = 10006,
	METHOD_PLAYER_LEFT = 10007, // player �ٴ� �ֵ��� �ٸ��ְ� ���Դ� �� �̷� ������
	METHOD_PLAYER_LIST = 10008,
	METHOD_MODERATOR_CHANGED = 10009,
	METHOD_START_GAME = 10010,
	METHOD_INDEX = 10011,
	METHOD_JOIN_PASSWORD_ROOM = 10012, // ����� �� ���� �� �޼���� ��.

	// 20000����� ���Ӱ� ���õ� �޼���
	METHOD_JOB = 20000,
	METHOD_TIME_CHANGED = 20001,
	METHOD_VOTE_RESULT = 20002,
	METHOD_FINAL_OBJECTION = 20003,
	METHOD_VOTE = 20004,
	METHOD_YESNO_VOTE = 20005,
	METHOD_VOTE_DEAD = 20006, // ��ǥ�� ó�������� ��
	METHOD_GAME_FINISHED = 20007,
	METHOD_HAND = 20008,
	// ���Ǿƿ� �ɷ��� �ʹ� ���Ƽ� �̷��� ���� �� ��.
	METHOD_SKILL = 20009,
	METHOD_INCREASE_TIME = 20010,
	METHOD_DECREASE_TIME = 20011,
	METHOD_MEMO = 20012,
	METHOD_CHANGE_HAND = 20013, // ���Ǿ� �ѱ� ��밡 �ٲ��� ��
	METHOD_STATUS_CHANGED = 20014, // �������� ���°� ������� / ������� �ٳ�.
	// ���� ������� 0, ���� 1
	METHOD_OTHER_HAND = 20015,
    
    // ���� ���� �޸��ϴ� �޼���
    METHOD_MEMO_CULTED = 20016
};
