#pragma once
#include <string>
#include <vector>
using namespace std;

// 욕설 필터
// 욕설을 ** 식으로 바꿔줌.
// 욕 파일을 읽어서 처리함
class ChatFilter
{
private:
	vector<pair<string, int>> dirty_word;

	char* replace_str;

public:
	void read(const string& filename);
	void filter(string& str);
	bool has_dirty_word(const string& str);
};

