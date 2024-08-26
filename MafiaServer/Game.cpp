#include "Game.h"
#include "Packet.h"
#include "Method.h"
#include "Utils.h"
#include "Forwarder.h"
#include "RandomEngine.h"

Game::Game(vector<Player>& p, int cnt, asio::io_context& context) :
	player(p),
	timer(context),
	player_cnt(cnt),
	time_status(TIME_NULL),
	skill(new Skill(this))
{
}

void Game::broadcast_if(Packet& packet, function<bool(Player&)> condition)
{
	for (auto& p : player)
	{
		if (p.is_empty)
			continue;

		if (!condition(p))
			continue;

		auto& s = p.session;
		s->write_packet(packet);
	}
}

void Game::broadcast(Packet& packet)
{
	broadcast_if(packet, [](Player&) -> bool { return true; });
}

void Game::forward(vector<Player>::iterator sender, const string& msg)
{
	Forwarder::forward(player, sender, msg, time_status);
}

void Game::memo(vector<Player>::iterator p, vector<Player>::iterator target)
{
	Packet packet;
	packet.method = METHOD_MEMO;
	packet.push(distance(player.begin(), target));
	packet.push(target->job.get_real_job());

	p->session->write_packet(packet);
}

void Game::memo_all(vector<Player>::iterator target)
{
	Packet packet;
	packet.method = METHOD_MEMO;
	packet.push(distance(player.begin(), target));
	packet.push(target->job.get_real_job());

	broadcast(packet);
}

void Game::memo_each(const vector<int>& list)
{
	for (int i = 0; i < list.size(); i++)
	{
		for (int j = 0; j < list.size(); j++)
		{
			if (i == j)
				continue;

			memo(next(player.begin(), list[i]),
				next(player.begin(), list[j]));
		}
	}
}

void Game::memo_culted(vector<Player>::iterator p, vector<Player>::iterator target)
{
    Packet packet;
    packet.method = METHOD_MEMO_CULTED;
    packet.push(distance(player.begin(), target));
    
    p->session->write_packet(packet);
}

void Game::memo_culted_each(const vector<int>& list)
{
    for (int i = 0; i < list.size(); i++)
    {
        for (int j = 0; j < list.size(); j++)
        {
            if (i == j)
                continue;

            memo_culted(next(player.begin(), list[i]),
                next(player.begin(), list[j]));
        }
    }
}

void Game::start()
{
	is_started = true;
	day_cnt = 0;

	for (auto& p : player)
		p.set_callback([](vector<Player>::iterator) {});

	for (auto& p : player)
		p.initialize();

	shuffle_job();

	Packet packet;
	packet.method = METHOD_JOB;

	for (auto& p : player)
	{
		if (p.is_empty)
			continue;

		p.is_used_skill = false;
		p.is_mafia_connected = false;
		p.vote_hand = player.end();

		packet.clear();
		packet.push(p.job.get_job());
		p.session->write_packet(packet);
	}

	// 짝, 짝 연인 누군지 알려줌.
	vector<int> mafia_list;
	vector<int> couple_list;
	vector<int> police_list;
	vector<int> doctor_list;
	for (int i = 0; i < player.size(); i++)
	{
		if (player[i].job.get_job() == MAFIA)
			mafia_list.push_back(i);
		if (player[i].job.get_job() == COUPLE)
			couple_list.push_back(i);
		if (player[i].job.get_job() == POLICE)
			police_list.push_back(i);
		if (player[i].job.get_job() == DOCTOR)
			doctor_list.push_back(i);
	}

	memo_each(mafia_list);
	memo_each(couple_list);
	memo_each(police_list);
	memo_each(doctor_list);

	timer.run(time_length[time_status], bind(&Game::on_time_changed, this));
}

/*void Game::shuffle_job()
{
    shuffle_job_new();
    return;
    
	int job_arr[] = { MAFIA, POLICE, DOCTOR, SPECIAL, SPECIAL, ASSIST,
		SPECIAL, MAFIA, CULT, SPECIAL, MAFIA, SPECIAL };
	// job_arr[i] : i인방에는 없었다가 i+1인방부터 새로 생기는 직업
	int special_cnt[] = { -1, 0, 0, 0, 1, 2, 2, 3, 3, 3, 4, 4, 5 };
	// special_cnt[i] : i인방 특수 직업 개수

	int special_arr[] = { SOLDIER, POLITICIAN, SHAMAN, REPORTER, GANGSTER, GHOUL,
		TERRORIST, DETECTIVE, PRIEST, COUPLE, COUPLE, MAGICIAN, HACKER, PROPHET,
		JUDGE, NURSE, MENTALIST };
	int special_size = 11;
	// 모든 특직, 섞어서 맨 앞에서부터 랜덤으로 꺼낼꺼임

	int assist_arr[] = { SPY, BEASTMAN, MADAM, THIEF, SCIENTIST, WITCH };

	shuffle(job_arr, job_arr + player_cnt, random_engine);
	//shuffle(special_arr, special_arr + 15, g);
	shuffle(special_arr, special_arr + special_size, random_engine);
	//swap(special_arr[0], special_arr[5]); // 도굴꾼 맨 앞에 나오게

	int couple_idx = -1;
	int couple_cnt = 0;
	for (int i = 0; i < special_cnt[player_cnt]; i++)
	{
		if (special_arr[i] == COUPLE)
		{
			couple_cnt++;
			couple_idx = i;
		}
	}

    // 이렇게 배정하면 5-6인방에서 연인 배정 확률이 1/55이다. 한명 걸리면 나머지 한명도 연인 되게 수정해야 함
	if (couple_cnt == 1)
	{
		cout << "One Couple!" << endl;

		int idx = special_cnt[player_cnt];
		while (special_arr[idx] == COUPLE)
			idx++;

		swap(special_arr[couple_idx], special_arr[idx]);
	}

	int special_idx = 0;

	//uniform_int_distribution<> dist1(0, 5);
	uniform_int_distribution<> dist1(0, 3); // 스파이, 짐승인간, 마담, 도둑
	int assist = assist_arr[dist1(random_engine)];

	int job_idx = 0;
	for (int i = 0; i < player.size(); i++)
	{
		auto& p = player[i];

		if (p.is_empty)
			continue;

		p.job.initialize(job_arr[job_idx]);
		job_idx++;

		if (p.job.get_job() == SPECIAL)
		{
			p.job.initialize(special_arr[special_idx++]);

			if (p.job.get_job() == POLITICIAN)
				p.vote_right = 2;
		}

		if (p.job.get_job() == ASSIST)
			p.job.initialize(assist); // 한 게임에 보조직은 최대 하나라서 이렇게 배정해도 됨. 
	}
}*/

void Game::shuffle_job()
{
    int job_arr[] = { MAFIA, POLICE, DOCTOR, SPECIAL, SPECIAL, ASSIST,
        SPECIAL, MAFIA, CULT, SPECIAL, MAFIA, SPECIAL };
    // job_arr[i] : i인방에는 없었다가 i+1인방부터 새로 생기는 직업
    int special_cnt[] = { -1, 0, 0, 0, 1, 2, 2, 3, 3, 3, 4, 4, 5 };
    // special_cnt[i] : i인방 특수 직업 개수

    int special_arr[] = { COUPLE, SOLDIER, POLITICIAN, SHAMAN, REPORTER, GANGSTER, GHOUL,
        TERRORIST, DETECTIVE, PRIEST, MAGICIAN, HACKER, PROPHET,
        JUDGE, NURSE, MENTALIST };
    int special_size = 10;
    // 모든 특직, 섞어서 맨 앞에서부터 랜덤으로 꺼낼꺼임

    int assist_arr[] = { SPY, BEASTMAN, MADAM, THIEF, SCIENTIST, WITCH };

    shuffle(job_arr, job_arr + player_cnt, random_engine);
    //shuffle(special_arr, special_arr + 15, g);
    
    // 4인방에서는 연인이 나오면 안됨
    if(player_cnt == 4)
    {
        swap(special_arr[0], special_arr[special_size-1]);
        shuffle(special_arr, special_arr + special_size - 1, random_engine);
    }
    else
        shuffle(special_arr, special_arr + special_size, random_engine);
    //swap(special_arr[0], special_arr[5]); // 도굴꾼 맨 앞에 나오게

    int couple_idx = -1;
    for (int i = 0; i < special_cnt[player_cnt]; i++)
    {
        if (special_arr[i] == COUPLE)
        {
            couple_idx = i;
        }
    }

    if (couple_idx != -1)
    {
        cout << "Couple!" << endl;
        
        int idx = 0;
        while (special_arr[idx] == COUPLE)
            idx++;

        special_arr[idx] = COUPLE;
    }

    int special_idx = 0;

    //uniform_int_distribution<> dist1(0, 5);
    uniform_int_distribution<> dist1(0, 3); // 스파이, 짐승인간, 마담, 도둑
    int assist = assist_arr[dist1(random_engine)];

    int job_idx = 0;
    for (int i = 0; i < player.size(); i++)
    {
        auto& p = player[i];

        if (p.is_empty)
            continue;

        p.job.initialize(job_arr[job_idx]);
        job_idx++;

        if (p.job.get_job() == SPECIAL)
        {
            p.job.initialize(special_arr[special_idx++]);

            if (p.job.get_job() == POLITICIAN)
                p.vote_right = 2;
        }

        if (p.job.get_job() == ASSIST)
            p.job.initialize(assist); // 한 게임에 보조직은 최대 하나라서 이렇게 배정해도 됨.
        
        if(p.job.get_job() == CULT)
            p.is_culted = true;
    }
}

void Game::change_time()
{
	time_status++;

	if (time_status == TIME_END)
		time_status = TIME_NIGHT;
}

// 시간이 바낄때 호출되는 콜백
// 시간을 밤으로 건너뛰고 싶으면 현재 시간을 TIME_NULL로 설정 후 0초 타이머 돌리면 됨.
void Game::on_time_changed()
{
	// 특정한 시간이 끝날때 호출
	on_time_ended();

	change_time();

	if (time_status == TIME_NIGHT)
		day_cnt++;

	skill->on_time_changed(time_status);

	int length = time_length[time_status];

	if (time_status == TIME_DAY)
	{
		// 살아있는 사람 수를 셈
		int alive_user = 0;

		for (auto& p : player)
		{
			if (p.is_empty)
				continue;

			if (p.status != PLAYER_ALIVE)
				continue;

			alive_user++;
		}

		length = alive_user * 15;
	}

	Packet packet;
	packet.method = METHOD_TIME_CHANGED;
	packet.push(time_status);
	packet.push(length);

	broadcast(packet);

	// 특정한 시간이 시작할때 호출
	on_time_started();

	int finish = check_finish();

	if (finish != NO_FINISH)
	{
		// 게임이 끝났음!

		packet.clear();
		packet.method = METHOD_GAME_FINISHED;
		packet.push(finish);

		for (auto& p : player)
		{
            if(p.job.get_initial_job() == Job_enum::NONE)
                continue;

			packet.push(p.nickname);
			packet.push(p.job.get_initial_job());
		}

		broadcast(packet);

		is_started = false;
		return;
	}

	timer.run(length, bind(&Game::on_time_changed, this));
}

void Game::on_time_started()
{
	switch (time_status)
	{
	case TIME_NIGHT:
		on_night_started();
		break;

	case TIME_DAY:
		on_day_started();
		break;

	case TIME_VOTE:
		on_vote_started();
		break;

	case TIME_FINAL_OBJECTION:
		break;

	case TIME_YESNO_VOTE:
		on_yesno_vote_started();
		break;
	}
}

void Game::on_time_ended()
{
	switch (time_status)
	{
	case TIME_NIGHT:
		break;

	case TIME_DAY:
		break;

	case TIME_VOTE:
		on_vote_ended();
		break;

	case TIME_FINAL_OBJECTION:
		break;

	case TIME_YESNO_VOTE:
		on_yesno_vote_ended();
		break;
	}
}

void Game::on_night_started()
{
	//day_cnt++;

	if (was_yesno)
	{
		was_yesno = false;
		//vote_kill();
	}

	for (auto it = player.begin(); it != player.end(); it++)
	{
		it->set_hand(player.end());
		it->is_threated = false;
		//it->is_used_skill = false;
		//it->is_tempted = false;
		it->is_modified_time = false;
	}
}

void Game::on_day_started()
{
	for (auto& p : player)
	{
		p.set_callback([](vector<Player>::iterator) {});
	}
}

void Game::on_vote_started()
{
	for (auto it = player.begin(); it != player.end(); it++)
	{
		it->job.reset_job();

		it->is_tempted = false;
		it->is_voted = false;
		it->is_final_objection = false;
		it->voted_cnt = 0;
		it->visible_voted_cnt = 0;
		it->vote_hand = player.end();
	}
}

void Game::on_vote_ended()
{
	// 투표 결과 나오기 전, 마담 도둑같은 애들 접선 처리
	for (auto it = player.begin(); it != player.end(); it++)
	{
		skill->vote_use(it, it->vote_hand);
	}

	// 투표시간이 끝났으니 투표 결과를 봐야 됨

	Packet packet;
	packet.method = METHOD_VOTE_RESULT;

	max_player = player.begin();

	for (auto it = player.begin(); it != player.end(); it++)
	{
		if (it->status == PLAYER_ALIVE && it->voted_cnt > 0)
		{
			packet.push(it->nickname);
			packet.push(it->visible_voted_cnt);
		}

		if (it->voted_cnt > max_player->voted_cnt)
			max_player = it;
	}

	broadcast(packet);

	int max_cnt = 0;

	for (auto& p : player)
	{
		if (p.voted_cnt == max_player->voted_cnt)
			max_cnt++;
	}

	if (max_cnt == 1)
	{
		max_player->is_final_objection = true;

		Packet packet;
		packet.method = METHOD_FINAL_OBJECTION;
		packet.push(max_player->nickname);

		broadcast(packet);

		//was_yesno = true;
		return;
	}
	else
	{
		time_status = TIME_NULL;
		return;
	}
}

void Game::on_final_objection_started()
{
}

void Game::on_yesno_vote_started()
{
	for (auto& p : player)
	{
		p.is_yesno_voted = false;
	}

	yes_cnt = 0;

	Packet packet;
	packet.method = METHOD_YESNO_VOTE;

	broadcast_if(packet, [](Player& p) -> bool {
		if (p.status != PLAYER_ALIVE)
			return false;

		if (p.is_threated)
			return false;

		return true;
	});
}

void Game::on_yesno_vote_ended()
{
	// 여기서 찬반 투표 결과 처리해서 처형시키던지 하면 됨.

	int player_cnt = 0;
	for (auto& p : player)
	{
		if (!p.is_empty && p.status == PLAYER_ALIVE)
			player_cnt += p.vote_right;
	}

	int no_cnt = player_cnt - yes_cnt;

	if (yes_cnt >= no_cnt) // 사형
	{
		skill->vote_kill(max_player);
	}
}

// https://namu.wiki/w/%EB%A7%88%ED%94%BC%EC%95%8442/%EA%B2%8C%EC%9E%84%20%EB%B0%A9%EB%B2%95 3.1
// 위 문서에 따르면 게임 종료를 이렇게 판정하면 안된다. 수정해야 함
int Game::check_finish()
{
    int mafia_cnt = 0, citizen_cnt = 0, cult_cnt = 0;
    bool is_cult_alive = false;
	for (auto& p : player)
	{
		if (p.is_empty)
			continue;
		if (p.status != PLAYER_ALIVE)
			continue;
		if (p.is_threated)
			continue;
        if(p.job.is_assist() && !p.is_connected_assist())
            continue;

		if (p.team() == MAFIA_TEAM)
			mafia_cnt += p.vote_right;
        else if(p.team() == CULT_TEAM)
            cult_cnt += p.vote_right;
		else
			citizen_cnt += p.vote_right;
        
        if(p.job.get_real_job() == CULT)
            is_cult_alive = true;
	}

	/*if (mafia_cnt >= citizen_cnt) // 마피아 투표권이 시민 투표권보다 같거나 많음. 마피아 승
		return MAFIA_WIN;
	else if (mafia_cnt == 0) // 마피아팀 몰살. 시민 승
        return CITIZEN_WIN;*/
    
    if(mafia_cnt >= citizen_cnt + cult_cnt)
        return MAFIA_WIN;
    
    else if(is_cult_alive && mafia_cnt == 0 && cult_cnt >= citizen_cnt)
        return CULT_WIN;
    else if(!is_cult_alive && mafia_cnt == 0 && citizen_cnt == 0)
        return CULT_WIN;
    
    else if(mafia_cnt == 0 && !is_cult_alive && citizen_cnt >= cult_cnt)
        return CITIZEN_WIN;

	return NO_FINISH;
}

void Game::increase_time(vector<Player>::iterator p)
{
	if (p->is_empty || p->is_modified_time ||
		p->status != PLAYER_ALIVE || time_status != TIME_DAY)
		return;

	p->is_modified_time = true;

	timer.increase_time(15);

	Packet packet;
	packet.method = METHOD_INCREASE_TIME;
	packet.push(p->nickname);
	broadcast(packet);
}

void Game::decrease_time(vector<Player>::iterator p)
{
	if (p->is_empty || p->is_modified_time ||
		p->status != PLAYER_ALIVE || time_status != TIME_DAY)
		return;

	p->is_modified_time = true;

	timer.decrease_time(15);

	Packet packet;
	packet.method = METHOD_DECREASE_TIME;
	packet.push(p->nickname);
	broadcast(packet);
}

bool Game::vote(vector<Player>::iterator p, vector<Player>::iterator target)
{
	if (time_status != TIME_VOTE)
		return false;

	if (p->is_empty || target->is_empty)
		return false;

	if (p->status != PLAYER_ALIVE)
		return false;

	if (target->status != PLAYER_ALIVE)
		return false;

	if (p->is_threated)
		return false;

	if (p->is_voted)
		return false;

	p->vote_hand = target;

	target->voted_cnt += p->vote_right;
	target->visible_voted_cnt++;
	p->is_voted = true;
	return true;
}

bool Game::yesno_vote(vector<Player>::iterator p, bool is_yes)
{
	if (time_status != TIME_YESNO_VOTE)
		return false;

	if (p->is_empty)
		return false;

	if (p->is_threated)
		return false;

	if (p->status != PLAYER_ALIVE)
		return false;

	if (p->is_yesno_voted)
		return false;

	p->is_yesno_voted = true;

	if(is_yes)
		yes_cnt += p->vote_right;

	return true;
}

bool Game::hand(vector<Player>::iterator p, vector<Player>::iterator target)
{
	if (p->is_empty)
		return false;

	if (p->status != PLAYER_ALIVE)
		return false;

	if (time_status == TIME_FINAL_OBJECTION &&
		p->job.get_job() == TERRORIST && p->is_final_objection)
		return skill->use(p, target);

	if (time_status != TIME_NIGHT)
		return false;

	return skill->use(p, target);
}
