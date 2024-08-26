#pragma once
#include <vector>
#include <string>
using namespace std;

class Utils
{
public:
	static void generate_random_str(string& res, int size = 30); // size ����Ʈ¥�� ���� ���ڿ��� ��������.
	static int generate_random_num(); // ������ ���ڸ� ��������.
	static void sha256_raw(string& res, const string& str); // SHA256 �ؽ��� ��� ����. ����Ʈ �迭�� res�� �����.

	static void wstring_to_string(const wstring& wstr, string& str);
	static void string_to_wstring(const string& str, wstring& wstr);

	static void push_int(vector<string>& vec, int value); // int�� ���� vector�� ���̳ʸ� �״�� �������
	static int bin_to_int(const string& str);
	static void int_to_bin(string& str, int value);

	static bool read_all_file(string& res, const string& filename); // filename ������ �о res�� �־���
	static bool is_valid_nickname(const string& nickname);

	static int str_length(const string& str);
};

