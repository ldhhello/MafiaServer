
// 시간 증가 / 시간 단축 기능이 있는 타이머
// 낮시간에 시간 단축 기능같은거 구현하려면 있어야 함.
// 시간 증가 / 시간 단축 메서드는 생성자에 인수로 들어온 context와 같은 쓰레드에서 실행해야 함.

#pragma once
#include "Include.h"
class Timer
{
private:
	asio::deadline_timer timer;
	function<void()> callback;
	bool is_running = false;

	int remain_time;

public:
	Timer(asio::io_context& context);
	~Timer();
	void run(int t, function<void()> c);

private:
	void run_callback(const boost::system::error_code& ec);

public:
	void increase_time(int t);
	void decrease_time(int t);
};

