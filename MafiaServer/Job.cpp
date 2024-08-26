#include "Job.h"

int Job::get_team(int job)
{
    if (is_special(job))
        return CITIZEN_TEAM;
    else if (is_assist(job))
        return MAFIA_TEAM;
    else
        return job_team[job];
}

Job::Job()
{
}

Job::Job(int j)
{
	initialize(j);
}

void Job::initialize(int j)
{
	job = real_job = initial_job = j;
	
    team = get_team(j);
}

int Job::get_job()
{
	return job;
}

int Job::get_real_job()
{
	return real_job;
}

int Job::get_initial_job()
{
    return initial_job;
}

void Job::set_job(int j)
{
	job = j;
}

void Job::reset_job()
{
	job = real_job;
}

void Job::set_real_job(int j)
{
	job = j;
	real_job = j;
    
    team = get_team(j);
}
