
// �ð� ���� / �ð� ���� ����� �ִ� Ÿ�̸�
// ���ð��� �ð� ���� ��ɰ����� �����Ϸ��� �־�� ��.
// �ð� ���� / �ð� ���� �޼���� �����ڿ� �μ��� ���� context�� ���� �����忡�� �����ؾ� ��.

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

