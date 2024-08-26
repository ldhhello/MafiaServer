#pragma once
#include <string>
#include <vector>
#include <functional>
using namespace std;

struct Packet
{
	int method;

private:
	vector<string> vec;

public:
	void to_string(string& data, bool has_packet_size = true) const; // 패킷 전체 사이즈까지 적어줌.
	bool write(function<bool(const void*, int)> write__, bool has_packet_size = true) const; // 소켓에 직접 Write 해줌.
	bool from_string(const string& data); // 패킷 전체 사이즈는 빼서 줘야함.

	void clear(); // 패킷에 있는 모든 데이터를 날림 (다음에 쓰기 위해), 메서드는 안날림
	void set_method(int m);
	int size();

	void push(int data);
	void push(const string& data);

	// set 메서드들은 idx 자리에 뭐가 없으면 오류
	void set(int idx, int data);
	void set(int idx, const string& data);

	int get_int(int idx);
	const string& get_string(int idx);
};