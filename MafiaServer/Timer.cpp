#include "Timer.h"

Timer::Timer(asio::io_context& context) :
	timer(context)
{
}

Timer::~Timer()
{
	is_running = false;
	timer.cancel();
}

void Timer::run(int t, function<void()> c)
{
	if (is_running)
		return;

	is_running = true;
	remain_time = t;
	callback = c;

	// 이게 없으면 0초짜리 타이머가 돌아가지 않아서 해야 됨.
	if (remain_time <= 0)
	{
		is_running = false;

		callback();
		return;
	}

	timer.expires_from_now(boost::posix_time::seconds(1));
	timer.async_wait(bind(&Timer::run_callback, this, placeholders::_1));
}

void Timer::run_callback(const boost::system::error_code& ec)
{
	if (ec.value() != 0)
		return;

	if (!is_running)
		return;

	remain_time--;

	if (remain_time <= 0)
	{
		is_running = false;

		callback();
		return;
	}

	timer.expires_from_now(boost::posix_time::seconds(1));
	timer.async_wait(bind(&Timer::run_callback, this, placeholders::_1));
}

void Timer::increase_time(int t)
{
	remain_time += t;
}

void Timer::decrease_time(int t)
{
	remain_time -= t;
}
