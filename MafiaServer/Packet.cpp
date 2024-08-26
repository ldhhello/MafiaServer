#include "Packet.h"
#include "Utils.h"
#include <string.h>
#include <arpa/inet.h>

void Packet::to_string(string& data, bool has_packet_size) const
// has_packet_size가 true면 전체 패킷 사이즈를 적어줌
{
	data.clear();

	int size = 12;

	if (!has_packet_size)
		size = 8;

	int vec_size = vec.size();
	for (auto& str : vec)
	{
		size += 4 + str.size();
	}

	data.resize(size);

	int buf_ = 0;

	int offset = 0;
	if (has_packet_size)
	{
		buf_ = htonl(size);
		memcpy(&data[0], &buf_, 4);
		offset = 4;
	}

	buf_ = htonl(method);
	memcpy(&data[offset], &buf_, 4);

	buf_ = htonl(vec_size);
	memcpy(&data[offset + 4], &buf_, 4);

	int pos = offset + 8;

	for (auto& str : vec)
	{
		int strsize = str.size();
		buf_ = htonl(strsize);
		memcpy(&data[pos], &buf_, 4);
		pos += 4;

		memcpy(&data[pos], str.c_str(), str.size());
		pos += str.size();
	}
}

bool Packet::write(function<bool(const void*, int)> write__, bool has_packet_size) const
{
	auto write_ = [&](const void* b, int s) -> void {
		if (!write__(b, s))
			throw exception();
	};

	try
	{
		int size = 12;

		if (!has_packet_size)
			size = 8;

		int vec_size = vec.size();
		for (auto& str : vec)
		{
			size += 4 + str.size();
		}

		int offset = 0;
		int buf_ = htonl(size);
		if (has_packet_size)
		{
			write_(&buf_, 4);
			offset = 4;
		}

		buf_ = htonl(method);
		write_(&buf_, 4);
		buf_ = htonl(vec_size);
		write_(&buf_, 4);

		int pos = offset + 8;

		for (auto& str : vec)
		{
			int strsize = str.size();
			buf_ = htonl(strsize);
			write_(&buf_, 4);
			pos += 4;

			write_(str.c_str(), str.size());
			pos += str.size();
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool Packet::from_string(const string& data)
{
	vec.clear();

	if (data.size() < 8) // 헤더 크기
		return false;

	int buf;

	memcpy(&buf, &data[0], 4);
	buf = ntohl(buf);
	method = buf;

	memcpy(&buf, &data[4], 4);
	buf = ntohl(buf);
	int data_size = buf;
    
    if(data_size < 0)
        return false;

	int offset = 8;
	for (int i = 0; i < data_size; i++)
	{
		int size;

		if (data.size() < offset + 4)
			return false;

		memcpy(&size, &data[offset], 4);
		size = ntohl(size);
		offset += 4;

		vec.emplace_back();
		string& str = vec.back();
        
        if(size < 0)
            return false;

		if (data.size() < offset + size)
			return false;

		str.resize(size);
		memcpy(&str[0], &data[offset], size);
		offset += size;
	}

	return true;
}

void Packet::clear()
{
	//method = 0;
	vec.clear();
}

void Packet::set_method(int m)
{
	method = m;
}

int Packet::size()
{
	return vec.size();
}

void Packet::push(int data)
{
	Utils::push_int(vec, data);
}

void Packet::push(const string& data)
{
	vec.push_back(data);
}

void Packet::set(int idx, int data)
{
	Utils::int_to_bin(vec[idx], data);
}

void Packet::set(int idx, const string& data)
{
	vec[idx] = data;
}

int Packet::get_int(int idx)
{
	return Utils::bin_to_int(vec[idx]);
}

const string& Packet::get_string(int idx)
{
	return vec[idx];
}
