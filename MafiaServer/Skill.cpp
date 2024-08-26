#include "Skill.h"
#include "Method.h"
#include "Utils.h"
#include "Strings.h"

// صباح الخير

void Skill::broadcast(Packet& p)
{
    game->broadcast(p);
}

Skill::Skill(Game* g) :
    game(g),
    mafia_gun_muzzle(g->player.end())
{
}

bool Skill::use(vector<Player>::iterator p, vector<Player>::iterator target)
{
    if (p->is_tempted)
        return false;

    switch (job_hand[p->job.get_job()])
    {
    case NO_HAND:
        return false;

    case FIXED_HAND:
        if (p->get_hand() != game->player.end())
            return false;

        if (use_fixed_hand(p, target))
        {
            p->set_hand(target);
            return true;
        }
        else
            return false;
        
    case MOVING_HAND:
        if (can_change_hand(p, target))
        {
            p->set_hand(target);

            if (p->job.get_job() == MAFIA || 
                (p->job.get_job() == BEASTMAN && p->is_connected_assist()))
            {
                mafia_gun_muzzle = target;

                Packet packet;
                packet.method = METHOD_CHANGE_HAND;
                packet.push(distance(game->player.begin(), mafia_gun_muzzle));

                for (auto it = game->player.begin(); it != game->player.end(); it++)
                {
                    if (it != p && 
                        (it->job.get_job() == MAFIA ||
                        (it->job.get_job() == BEASTMAN && it->is_connected_assist())))
                        it->session->write_packet(packet);
                }
            }

            return true;
        }
        return false;
    }

    return false;
}

void Skill::vote_use(vector<Player>::iterator p, vector<Player>::iterator target)
{
    Packet packet;
    packet.method = METHOD_SKILL;

    switch (p->job.get_job())
    {
    case MADAM:
        if (target == game->player.end())
            break;

        if (target->job.get_job() == MAFIA)
        {
            connect(p);
            break;
        }

        target->is_tempted = true;
        packet.clear();
        packet.push(SKILL_GOT_TEMPTED);
        target->session->write_packet(packet);

        packet.clear();
        packet.push(SKILL_MADAM_TEMPT);
        packet.push(target->nickname);

        p->session->write_packet(packet);

        break;

    case THIEF:
        if (target == game->player.end())
            break;

        if (target->job.get_job() == SOLDIER)
        {
            packet.clear();
            packet.push(SKILL_THIEF_STEAL_SOLDIER);
            p->session->write_packet(packet);
            
            packet.clear();
            packet.push(SKILL_SOLDIER_BLOCK_THIEF);
            packet.push(p->nickname);
            target->session->write_packet(packet);
            break;
        }
            
        if(target->job.get_job() == CULT)
        {
            packet.clear();
            
            // p가 도둑, target가 교주
            p->is_culted = true;
                
            game->memo(p, target);
            game->memo(target, p);
            
            game->memo_culted(target, p);
            game->memo_culted(p, target);
            game->memo_culted(p, p);
                
            packet.push(SKILL_CULT_RECRUIT);
            packet.push(p->nickname);
            target->session->write_packet(packet);
            
            packet.clear();
            packet.push(SKILL_GOT_CULTED);
            packet.push(target->nickname);
            p->session->write_packet(packet);
                
            packet.clear();
            packet.push(SKILL_CULT_RINGS);
            broadcast(packet);
        }

        p->job.set_job(target->job.get_job());

        packet.clear();
        packet.push(SKILL_THIEF_STEAL);
        packet.push(target->nickname);
        packet.push(job_name[target->job.get_job()]);
        p->session->write_packet(packet);
            
        game->memo(p, target);

        if (target->job.get_job() == MAFIA)
        {
            connect(p);
            break;
        }
    }
}

void Skill::on_time_changed(int t)
{
    switch (t)
    {
    case TIME_DAY:
        mafia_kill();
        mafia_gun_muzzle = game->player.end();

        on_day_started();
        break;
    }
}

bool Skill::can_change_hand(vector<Player>::iterator p, vector<Player>::iterator target)
{
    switch (p->job.get_job())
    {
    case MAFIA:
    case DOCTOR:
    case TERRORIST:
    case BEASTMAN:
        return target->status == PLAYER_ALIVE;

    case REPORTER:
        if (game->day_cnt == 1)
        {
            // 엠바고 (첫번째 날에는 취재할수 없습니다)
            Packet packet;
            packet.method = METHOD_ERROR;
            packet.push(STRING_CANT_REPORT_FIRST_DAY);
            p->session->write_packet(packet);
        }

        return !p->is_used_skill && game->day_cnt != 1 && target->status == PLAYER_ALIVE;
    case PRIEST:
        // 성불 당한 사람도 손은 올릴수 있어야 함.
        return !p->is_used_skill && target->status != PLAYER_ALIVE;
    }

    return false;
}

bool Skill::use_fixed_hand(vector<Player>::iterator p, vector<Player>::iterator target)
{
    Packet packet;
    packet.method = METHOD_SKILL;

    switch (p->job.get_job())
    {
    case POLICE:
        if (target->status != PLAYER_ALIVE)
            return false;

        if (target->job.get_job() == MAFIA && target->job.get_real_job() != THIEF)
            packet.push(SKILL_CAUGHT_MAFIA);
        else
            packet.push(SKILL_NO_MAFIA);

        packet.push(target->nickname);
        p->session->write_packet(packet);

        if (target->job.get_job() == MAFIA)
        {
            game->memo(p, target);
        }
        return true;

    case SHAMAN:
        if (target->status == PLAYER_ALIVE)
            return false;

        target->status = PLAYER_SEONGBUL;

        packet.push(SKILL_SEONGBUL);
        packet.push(target->nickname);
        packet.push(job_name[target->job.get_job()]);
        p->session->write_packet(packet);

        game->memo(p, target);

        packet.clear();
        packet.push(SKILL_GOT_SEONGBULLED);
        target->session->write_packet(packet);

        return true;
    case GANGSTER:
        if (target->status != PLAYER_ALIVE)
            return false;

        target->is_threated = true;

        packet.push(SKILL_GANGSTER_THREATEN);
        packet.push(target->nickname);
        p->session->write_packet(packet);

        packet.clear();
        packet.push(SKILL_GOT_THREATENED);
        target->session->write_packet(packet);

        return true;
    case SPY:
        if (target->status != PLAYER_ALIVE)
            return false;

        if (target->job.get_job() == SOLDIER)
        {
            packet.push(SKILL_SOLDIER_BLOCK_SPY);
            packet.push(p->nickname);
            target->session->write_packet(packet);

            packet.clear();
            packet.push(SKILL_SPY_GOT_DISCOVERED);
            packet.push(target->nickname);
            target->session->write_packet(packet);
            return true;
        }

        packet.push(SKILL_JOB);
        packet.push(target->nickname);
        packet.push(job_name[target->job.get_job()]);
        p->session->write_packet(packet);

        game->memo(p, target);

        if (target->job.get_job() == MAFIA)
            connect(p);

        return true;
    case DETECTIVE:
        if (target->status != PLAYER_ALIVE)
            return false;

        target->set_callback(
            bind(&Skill::on_other_hand, this, p, placeholders::_1)
        );

        packet.push(SKILL_DETECTIVE);
        packet.push(target->nickname);
        p->session->write_packet(packet);

        return true;
            
    case CULT:
        if (target->status != PLAYER_ALIVE)
            return false;
        
        if(game->day_cnt % 2 != 1) // 홀수밤에만 포교
            return false;
        
        if(target->job.get_real_job() == MAFIA) // 교크
        {
            game->memo(p, target);
            
            packet.push(SKILL_CULT_RECRUIT_FAILED);
            packet.push(job_name[target->job.get_real_job()]);
            packet.push(target->nickname);
            p->session->write_packet(packet);
            
            return true;
        }
        
        if(target->job.get_real_job() == PRIEST) // 성크
        {
            game->memo(p, target);
            game->memo(target, p);
            
            packet.push(SKILL_CULT_RECRUIT_FAILED);
            packet.push(job_name[target->job.get_real_job()]);
            packet.push(target->nickname);
            p->session->write_packet(packet);
            
            packet.clear();
            packet.push(SKILL_PRIEST_BLOCK_CULT);
            packet.push(p->nickname);
            target->session->write_packet(packet);
            
            return true;
        }
        
        target->is_culted = true;
            
        game->memo(p, target);
        game->memo(target, p);
        
        game->memo_culted(p, target);
        game->memo_culted(target, p);
        game->memo_culted(target, target);
            
        packet.push(SKILL_CULT_RECRUIT);
        packet.push(target->nickname);
        p->session->write_packet(packet);
        
        packet.clear();
        packet.push(SKILL_GOT_CULTED);
        packet.push(p->nickname);
        target->session->write_packet(packet);
            
        packet.clear();
        packet.push(SKILL_CULT_RINGS);
        broadcast(packet);
            
        return true;
    }

    return false;
}

void Skill::connect(vector<Player>::iterator p)
{
    p->is_mafia_connected = true;

    Packet packet;
    packet.method = METHOD_SKILL;
    packet.push(SKILL_CONNECT);

    p->session->write_packet(packet);

    for (int i = 0; i < game->player.size(); i++)
    {
        auto& p2 = game->player[i];

        if (p2.job.get_job() == MAFIA)
        {
            p2.session->write_packet(packet);
            game->memo(next(game->player.begin(), i), p);
            game->memo(p, next(game->player.begin(), i));
        }
    }
}

void Skill::mafia_kill()
{
    Packet packet;
    packet.method = METHOD_SKILL;

    // 마피아가 총을 쏘지 않았음
    if (mafia_gun_muzzle == game->player.end())
    {
        packet.clear();
        packet.push(SKILL_NOTHING_HAPPENED);
        broadcast(packet);
        return;
    }

    // 짐인 접 성공
    auto beastman_hand = get_beastman_hand();
    if (mafia_gun_muzzle->job.get_job() == BEASTMAN ||
        mafia_gun_muzzle == beastman_hand)
    {
        for (auto it = game->player.begin(); it != game->player.end(); it++)
        {
            if (it->job.get_job() == BEASTMAN)
                connect(it);
        }

        if (mafia_gun_muzzle->job.get_job() == BEASTMAN)
        {
            packet.clear();
            packet.push(SKILL_NOTHING_HAPPENED);
            broadcast(packet);
            return;
        }
    }

    // 짐인 킬 (짐인 손이랑 마피아 총구랑 같을 때)
    if (mafia_gun_muzzle == beastman_hand)
    {
        mafia_gun_muzzle->status = PLAYER_DEAD;

        packet.clear();
        packet.push(SKILL_EATEN_BY_BEAST);
        packet.push(mafia_gun_muzzle->nickname);
        broadcast(packet);

        ghoul();

        send_status(mafia_gun_muzzle);
        return;
    }

    // 의사가 치료에 성공했음
    if (is_doctor_heal(mafia_gun_muzzle))
    {
        packet.clear();
        packet.push(SKILL_DOCTOR_HEAL);
        packet.push(mafia_gun_muzzle->nickname);
        broadcast(packet);
        return;
    }

    // 군인이 공격을 버텨냈음
    if (mafia_gun_muzzle->job.get_job() == SOLDIER && !mafia_gun_muzzle->is_used_skill)
    {
        mafia_gun_muzzle->is_used_skill = true;

        packet.clear();
        packet.push(SKILL_SOLDIER_BLOCK);
        packet.push(mafia_gun_muzzle->nickname);
        broadcast(packet);

        game->memo_all(mafia_gun_muzzle);
        return;
    }

    // 테러 터지기 성공
    if (mafia_gun_muzzle->job.get_job() == TERRORIST &&
        mafia_gun_muzzle->get_hand()->job.get_job() == MAFIA)
    {
        mafia_gun_muzzle->status = PLAYER_DEAD;
        mafia_gun_muzzle->get_hand()->status = PLAYER_DEAD;

        packet.clear();
        packet.push(SKILL_TERRORIST_MAFIA_EXPLODE);
        packet.push(mafia_gun_muzzle->nickname);
        packet.push(mafia_gun_muzzle->get_hand()->nickname);
        broadcast(packet);

        send_status(mafia_gun_muzzle);
        send_status(mafia_gun_muzzle->get_hand());

        game->memo_all(mafia_gun_muzzle);
        game->memo_all(mafia_gun_muzzle->get_hand());
        return;
    }

    if (mafia_gun_muzzle->job.get_real_job() == COUPLE)
    {
        auto match_couple = game->player.begin();

        for (; match_couple != game->player.end(); match_couple++)
        {
            if (match_couple->is_empty || match_couple->status != PLAYER_ALIVE)
                continue;

            if (mafia_gun_muzzle != match_couple && 
                match_couple->job.get_real_job() == COUPLE)
                break;
        }

        if (match_couple != game->player.end())
        {
            packet.clear();
            packet.push(SKILL_COUPLE_SACRIFICE);
            packet.push(match_couple->nickname);
            packet.push(mafia_gun_muzzle->nickname);

            broadcast(packet);

            game->memo_all(mafia_gun_muzzle);
            game->memo_all(match_couple);

            mafia_gun_muzzle = match_couple;
            mafia_gun_muzzle->status = PLAYER_DEAD;

            send_status(mafia_gun_muzzle);

            ghoul();
            return;
        }
    }

    // 마피아 킬 성공
    packet.clear();
    packet.push(SKILL_MAFIA_KILL);
    packet.push(mafia_gun_muzzle->nickname);
    mafia_gun_muzzle->status = PLAYER_DEAD;

    broadcast(packet);

    send_status(mafia_gun_muzzle);

    ghoul();
}

void Skill::ghoul()
{
    Packet packet;
    packet.method = METHOD_SKILL;

    // 도굴꾼이 있음
    vector<Player>::iterator ghoul = game->player.begin();
    for (; ghoul != game->player.end(); ghoul++)
        if (!ghoul->is_empty && ghoul->status == PLAYER_ALIVE &&
            ghoul->job.get_job() == GHOUL)
            break;

    if (game->day_cnt == 1 && ghoul != game->player.end())
    {
        ghoul->job.set_real_job(mafia_gun_muzzle->job.get_job());
        mafia_gun_muzzle->job.set_real_job(
            mafia_gun_muzzle->team() == CITIZEN_TEAM ?
            CITIZEN : VILLAIN);

        // 정치인 같은 직업을 위해
        swap(mafia_gun_muzzle->vote_right, ghoul->vote_right);

        packet.clear();
        packet.push(SKILL_GRAVE_ROB);
        packet.push(job_name[ghoul->job.get_real_job()]);
        ghoul->session->write_packet(packet);

        packet.clear();
        packet.push(SKILL_GOT_ROBBED);
        packet.push(job_name[mafia_gun_muzzle->job.get_real_job()]);
        mafia_gun_muzzle->session->write_packet(packet);

        game->memo(ghoul, ghoul);
        game->memo(ghoul, mafia_gun_muzzle);
        game->memo(mafia_gun_muzzle, mafia_gun_muzzle);
        
        // 도굴꾼이 마피아/연인 직업을 획득한 경우 - 상대 마피아/연인이 누구인지 메모한다
        if(ghoul->job.get_real_job() == MAFIA)
        {
            vector<int> mafia_list;
            for (int i = 0; i < game->player.size(); i++)
            {
                if (game->player[i].job.get_job() == MAFIA)
                    mafia_list.push_back(i);
            }

            game->memo_each(mafia_list);
        }
        else if(ghoul->job.get_real_job() == COUPLE)
        {
            vector<int> couple_list;
            for (int i = 0; i < game->player.size(); i++)
            {
                if (game->player[i].job.get_job() == COUPLE)
                    couple_list.push_back(i);
            }

            game->memo_each(couple_list);
        }
    }
}

void Skill::send_status(vector<Player>::iterator p)
{
    Packet packet;
    packet.method = METHOD_STATUS_CHANGED;

    packet.push(distance(game->player.begin(), p));
    packet.push(p->status == PLAYER_ALIVE ? PLAYER_ALIVE : PLAYER_DEAD);
    broadcast(packet);
}

void Skill::on_day_started()
{
    Packet packet;
    packet.method = METHOD_SKILL;

    for (auto& p : game->player)
    {
        if (p.status != PLAYER_ALIVE)
            continue;

        if (job_hand[p.job.get_job()] != MOVING_HAND)
            continue;

        if (p.get_hand() == game->player.end() || p.get_hand()->is_empty)
            continue;

        switch (p.job.get_job())
        {
        case REPORTER:
            if (p.get_hand()->status != PLAYER_ALIVE)
            {
                packet.clear();
                packet.push(SKILL_HEADLINE_FAILED);
                p.session->write_packet(packet);
            }
            else
            {
                packet.clear();
                packet.push(SKILL_HEADLINE);
                packet.push(p.get_hand()->nickname);
                packet.push(job_name[p.get_hand()->job.get_real_job()]);
                broadcast(packet);

                game->memo_all(p.get_hand());
            }

            p.is_used_skill = true;
            break;
        case PRIEST:
            if (p.get_hand()->status == PLAYER_SEONGBUL)
            {
                packet.clear();
                packet.push(SKILL_REVIVE_FAILED);
                p.session->write_packet(packet);
            }
            else
            {
                p.get_hand()->status = PLAYER_ALIVE;

                packet.clear();
                packet.push(SKILL_REVIVE);
                packet.push(p.get_hand()->nickname);

                broadcast(packet);

                send_status(p.get_hand());
            }

            p.is_used_skill = true;
            break;
        }
    }
}

bool Skill::is_doctor_heal(vector<Player>::iterator target)
{
    for (auto& p : game->player)
    {
        if (p.is_empty)
            continue;

        if (p.job.get_job() != DOCTOR)
            continue;

        if (p.get_hand() == game->player.end())
            continue;

        // 의사 힐 적중!
        if (p.get_hand() == target)
            return true;
    }

    return false;
}

vector<Player>::iterator Skill::get_beastman_hand()
{
    for (auto& p : game->player)
    {
        if (p.job.get_job() == BEASTMAN)
        {
            return p.get_hand();
        }
    }

    return game->player.end();
}

void Skill::vote_kill(vector<Player>::iterator p)
{
    if (p->job.get_job() == POLITICIAN)
    {
        Packet packet;
        packet.method = METHOD_SKILL;
        packet.push(SKILL_POLITICIAN);
        packet.push(job_name[p->job.get_job()]);

        broadcast(packet);

        game->memo_all(p);
        return;
    }
    if (p->job.get_job() == TERRORIST)
    {
        if (p->get_hand() == game->player.end() || p->is_empty)
            return;

        p->status = PLAYER_DEAD;
        p->get_hand()->status = PLAYER_DEAD;

        Packet packet;
        packet.method = METHOD_SKILL;
        packet.push(SKILL_TERRORIST_VOTE_EXPLODE);
        packet.push(p->nickname);
        packet.push(p->get_hand()->nickname);

        broadcast(packet);

        send_status(p);
        send_status(p->get_hand());

        game->memo_all(p);
        return;
    }

    p->status = PLAYER_DEAD;

    Packet packet;
    packet.method = METHOD_VOTE_DEAD;
    packet.push(p->nickname);

    send_status(p);

    broadcast(packet);
}

void Skill::on_other_hand(vector<Player>::iterator detective, vector<Player>::iterator hand)
{
    Packet packet;
    packet.method = METHOD_OTHER_HAND;
    packet.push(distance(game->player.begin(), hand));

    detective->session->write_packet(packet);
}
