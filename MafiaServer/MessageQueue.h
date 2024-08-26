
// �� �̰� ��� ���� �Ǵµ� �̰� �� �ִ°���
// �ϴ� �ڵ�� ����

#if 0

#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

template<typename T>
class MessageQueue
{
private:
	condition_variable cv;
	queue<T> q;
	mutex m;

public:
	void enqueue(const T& data)
	{
		unique_lock<mutex> lock(m);
		q.push(data);
		cv.notify_one();
	}

	void dequeue(T& data)
	{
		unique_lock<mutex> lock(m);
		while (q.empty()) {
			cv.wait(lock);
		}
		data = q.front();
		q.pop();
	}

	T dequeue() { T data; dequeue(data); return data; }
};

#endif