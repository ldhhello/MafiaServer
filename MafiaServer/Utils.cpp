#include "Utils.h"
#include <random>
#include <openssl/sha.h>
#include <iconv.h>
#include <boost/locale.hpp>
#include <fstream>
#include <arpa/inet.h>

random_device rd;

const string displayable =
R"(1234567890-=qwertyuiop[]\asdfghjkl;zxcvbnm@#$%^&*_+QAZXSWEDCVFRTGBNHYUJMKIOL:P|)";
void Utils::generate_random_str(string& res, int size)
{
	res.clear();

	mt19937 gen(rd());

	uniform_int_distribution<int> dis1(20, 30);
	res.reserve(size);

	uniform_int_distribution<int> dis2(0, displayable.size() - 1);

	for (int i = 0; i < size; i++)
	{
		res.push_back(displayable[dis2(gen)]);
	}
}

int Utils::generate_random_num()
{
	mt19937 gen(rd());

	uniform_int_distribution<int> dis(INT_MIN, INT_MAX);
	return dis(gen);
}

void Utils::sha256_raw(string& res, const string& str)
{
	res.clear();
	res.resize(SHA256_DIGEST_LENGTH);
	SHA256((const unsigned char*)str.c_str(), str.length(), (unsigned char*)&res[0]);
}

void Utils::wstring_to_string(const wstring& wstr, string& str)
{
	str = boost::locale::conv::utf_to_utf< char, wchar_t >(wstr);

	//출처: https://dev-skill.tistory.com/121 [일타필성! 개발신공!]
}

void Utils::string_to_wstring(const string& str, wstring& wstr)
{
	wstr = boost::locale::conv::utf_to_utf< wchar_t, char >(str);

	//출처: https://dev-skill.tistory.com/121 [일타필성! 개발신공!]
}

void Utils::push_int(vector<string>& vec, int value)
{
	value = htonl(value);

	vec.emplace_back(4, 0);
	memcpy(&vec.back()[0], &value, 4);
}

int Utils::bin_to_int(const string& str)
{
	if (str.size() != 4)
		return 0; // str에 int로 변환할수 없는게 들어오면 오류임
	// 예외 던지면 코드 짜기 너무 힘들어짐. 걍 0 던질꺼임

	const int* ptr = (const int*)str.c_str();
	int res = *ptr;
	return ntohl(res);
}

void Utils::int_to_bin(string& str, int value)
{
	value = htonl(value);

	str.resize(4);
	memcpy(&str[0], &value, 4);
}

bool Utils::read_all_file(string& res, const string& filename)
{
	ifstream fin(filename, ios::binary);

	if (fin.fail())
		return false;

	fin.seekg(0, ios::end);
	int length = fin.tellg();
	fin.seekg(0, ios::beg);

	res.resize(length);

	fin.read(&res[0], length);

	return true;
}

bool Utils::is_valid_nickname(const string& nickname)
{
	wstring wstr;
	string_to_wstring(nickname, wstr);

	if (wstr.size() == 0 || wstr.size() > 6)
		return false;

	for (auto& ch : wstr)
	{
		if (!(44032 <= ch && ch <= 55203) && // 한글 (가 ~ 힣) 이 아님
			!(L'0' <= ch && ch <= L'9') && // 숫자가 아님
			!(L'a' <= ch && ch <= L'z') && // 알파벳 소문자가 아님
			!(L'A' <= ch && ch <= L'Z')) // 알파벳 대문자가 아님
			return false;
	}

	return true;
}

int Utils::str_length(const string& str)
{
	wstring wstr;
	try
	{
		string_to_wstring(str, wstr);
	}
	catch (...) {}

	return wstr.length();
}
