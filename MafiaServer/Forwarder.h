#pragma once
#include "Game.h"

class Forwarder
{
public:
	// 게임 중 현재 시간이 t고 sender가 str 라는 말을 보냈을때 채팅을 다른 사람들에게 포워딩 해줌.
	static void forward(vector<Player>& player, vector<Player>::iterator sender, const string& msg, int t);
	static void forward_if (
		vector<Player>& player,
		vector<Player>::iterator sender,
		const string& msg,
		function<int(vector<Player>::iterator)> check
	);
};

