#pragma once
#include "Include.h"
// صباح الخير

// 일단 지금은 시즌5 클래식 직업만 다 구현할꺼임.
// 그래도 일단 직업 리스트에 시즌8 직업 전체 (광신도 제외) 넣어놨음

enum Team
{
	NO_TEAM,
	MAFIA_TEAM,
	CITIZEN_TEAM,
	CULT_TEAM // 교주는 안만들껀데 그래도 일단 넣어놨음)
};

enum Job_hand
{
	NO_HAND,
	FIXED_HAND,
	MOVING_HAND,
	VOTE_HAND // 마담 도둑 같은 직업
};

enum Job_enum
{
	NONE = 0, MAFIA, POLICE, DOCTOR, SPECIAL, ASSIST, CULT,
	CITIZEN, SOLDIER, POLITICIAN, SHAMAN, REPORTER, 
	GANGSTER, DETECTIVE, GHOUL, TERRORIST, PRIEST, 
	MAGICIAN, HACKER, PROPHET, JUDGE, NURSE, MENTALIST, COUPLE,
	VILLAIN, SPY, BEASTMAN, MADAM, THIEF, SCIENTIST, WITCH,
    FROG
};

const int job_team[] =
{
	NO_TEAM, MAFIA_TEAM, CITIZEN_TEAM, CITIZEN_TEAM,
	CITIZEN_TEAM, MAFIA_TEAM, CULT_TEAM
};

const string job_name[] =
{
	"무직", "마피아", "경찰", "의사", "특직", "보조직", "교주",
	"시민", "군인", "정치인", "영매", "기자",
	"건달", "사립탐정", "도굴꾼", "테러리스트", "성직자", 
	"마술사", "해커", "예언자", "판사", "간호사", "심리학자", "연인",
	"악인", "스파이", "짐승인간", "마담", "도둑", "과학자", "마녀",
    "개구리"
};

const int job_hand[] =
{
	NO_HAND, MOVING_HAND, FIXED_HAND, MOVING_HAND, NO_HAND, NO_HAND, FIXED_HAND,
	NO_HAND, NO_HAND, NO_HAND, FIXED_HAND, MOVING_HAND,
	FIXED_HAND, FIXED_HAND, NO_HAND, MOVING_HAND, MOVING_HAND,
	FIXED_HAND, NO_HAND, NO_HAND, NO_HAND, MOVING_HAND, FIXED_HAND, NO_HAND,
	NO_HAND, FIXED_HAND, MOVING_HAND, VOTE_HAND, VOTE_HAND, NO_HAND, NO_HAND,
    NO_HAND
};

class Job
{
private:
    friend class Player;
    
	int job = 0;
	int real_job = 0; // 도둑, 마술사 등등 직업이 바끼는 직업들을 위해 있음)
    
    int initial_job = 0; // 마지막 직업 공개 때 공개될 직업 (도굴꾼이 제대로 안뜬다..)
    
    int get_team(int job);
    
    bool is_special(int job) { return job >= CITIZEN && job < VILLAIN; }
    bool is_assist(int job) { return job >= VILLAIN && job < FROG; }

private:
	int team = 0; // 팀이 바끼는 직업들을 위해 있음)

public:
	Job();
	Job(int j);
	void initialize(int j);

	int get_job();
	int get_real_job();
    
    int get_initial_job();
    
	void set_job(int j); // 진짜 직업은 안바꿈
	void reset_job(); // 현재 직업을 진짜 직업으로 바꿈
	void set_real_job(int j); // 도굴꾼같이 진짜 직업 바꾸는 애때매 필요함.

    bool is_special() { return is_special(job); }
    bool is_assist() { return is_assist(job); }
};

