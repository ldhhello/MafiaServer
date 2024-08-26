#pragma once
#include <vector>
#include <string>
using namespace std;

class Utils
{
public:
	static void generate_random_str(string& res, int size = 30); // size 바이트짜리 랜덤 문자열을 생성해줌.
	static int generate_random_num(); // 랜덤한 숫자를 생성해줌.
	static void sha256_raw(string& res, const string& str); // SHA256 해싱한 결과 리턴. 바이트 배열이 res에 저장됨.

	static void wstring_to_string(const wstring& wstr, string& str);
	static void string_to_wstring(const string& str, wstring& wstr);

	static void push_int(vector<string>& vec, int value); // int형 값을 vector에 바이너리 그대로 집어넣음
	static int bin_to_int(const string& str);
	static void int_to_bin(string& str, int value);

	static bool read_all_file(string& res, const string& filename); // filename 파일을 읽어서 res에 넣어줌
	static bool is_valid_nickname(const string& nickname);

	static int str_length(const string& str);
};

