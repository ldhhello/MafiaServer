#include "Player.h"

void Player::initialize()
{
	//job.reset_job();
    job.initialize(Job_enum::NONE);
    
	is_threated = false;
	is_used_skill = false;
	is_tempted = false;
	is_voted = false;
	is_yesno_voted = false;
	is_mafia_connected = false;
	is_modified_time = false;
	is_final_objection = false;
	voted_cnt = 0;
	visible_voted_cnt = 0;
    is_culted = false;
	vote_right = 1;
	status = PLAYER_ALIVE;
}

void Player::set_hand(vector<Player>::iterator h)
{
	hand = h;

	callback(hand);
}

vector<Player>::iterator Player::get_hand()
{
	return hand;
}

void Player::set_callback(function<void(vector<Player>::iterator)> f)
{
	callback = f;
	callback(hand);
}

bool Player::is_connected_assist()
{
	if (!job.is_assist())
		return false;

	if (!is_mafia_connected)
		return false;

	return true;
}

int Player::team()
{
    if(is_culted)
        return CULT_TEAM;
    
	return job.team;
}
