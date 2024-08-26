#include "Forwarder.h"
#include "Method.h"
#include "Utils.h"

#define FORWARD_CONDITION(c) ([](vector<Player>::iterator it) -> int { return (c); })
#define FORWARD_IF(c) forward_if(player, sender, msg, FORWARD_CONDITION(c))

void Forwarder::forward(vector<Player>& player, vector<Player>::iterator sender, const string& msg, int t)
{
    if(sender->status == PLAYER_SEONGBUL)
        return;
    
	// 죽은 사람 채팅
	if (sender->status == PLAYER_DEAD)
		FORWARD_IF(((it->job.get_job() == SHAMAN && !it->is_tempted) ||
			it->status != PLAYER_ALIVE)
			? Room::CHAT_DEAD : Room::CHAT_NULL);

	// 낮 채팅
	else if (sender->status == PLAYER_ALIVE &&
		(t == TIME_DAY || t == TIME_VOTE || t == TIME_YESNO_VOTE))
		FORWARD_IF(Room::CHAT_NORMAL);

	// 최후의 반론 채팅
	else if (t == TIME_FINAL_OBJECTION && sender->is_final_objection)
		FORWARD_IF(Room::CHAT_NORMAL);

	// 마피아 / 접선한 보조직 채팅
	else if (t == TIME_NIGHT &&
		(sender->job.get_job() == MAFIA || sender->is_connected_assist()))
		FORWARD_IF((it->job.get_job() == MAFIA || it->is_connected_assist() ||
			it->status != PLAYER_ALIVE)
			? Room::CHAT_MAFIA : Room::CHAT_NULL);

	// 영매 채팅
	else if (t == TIME_NIGHT &&
		sender->job.get_job() == SHAMAN)
		FORWARD_IF((it->status != PLAYER_ALIVE || it->job.get_job() == SHAMAN) ?
			Room::CHAT_NORMAL : Room::CHAT_NULL);

	// 연인 채팅
	else if (t == TIME_NIGHT &&
		sender->job.get_job() == COUPLE)
		FORWARD_IF((it->status != PLAYER_ALIVE || it->job.get_job() == COUPLE) ?
			Room::CHAT_COUPLE : Room::CHAT_NULL);
    
    // 교주 채팅
    else if(t == TIME_NIGHT &&
        sender->job.get_job() == CULT)
        FORWARD_IF((it->status != PLAYER_ALIVE || it->is_culted) ?
            Room::CHAT_CULT : Room::CHAT_NULL);
}

void Forwarder::forward_if (
	vector<Player>& player, 
	vector<Player>::iterator sender, 
	const string& msg, 
	function<int(vector<Player>::iterator)> check)
{
	Packet packet;
	packet.method = METHOD_CHAT;

	packet.push(0);
	packet.push(sender->session->nickname);
	packet.push(msg);

	for (auto it = player.begin(); it != player.end(); it++)
	{
		if (it->is_empty)
			continue;

		int chat_type = check(it);

		if (chat_type != Room::CHAT_NULL)
		{
			packet.set(0, chat_type);
			it->session->write_packet(packet);
		}
	}
}
