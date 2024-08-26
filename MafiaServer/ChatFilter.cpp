#include "ChatFilter.h"
#include "Utils.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

void ChatFilter::read(const string& filename)
{
	string file;
	Utils::read_all_file(file, filename);

	vector<string> vec;


	boost::split(vec, file, [](char c) -> bool
	{
		return c == '\n';
	});

	dirty_word.reserve(vec.size());
	for (auto& s : vec)
	{
		if(s.size() == 0)
			continue;

		dirty_word.emplace_back(s, Utils::str_length(s));
	}

	sort(dirty_word.begin(), dirty_word.end(), 
		[](const pair<string, int>& s1, const pair<string, int>& s2) -> bool {
			return s1.second > s2.second;
		}
	);

	int max_length = dirty_word[0].second;

	for (auto& s : dirty_word)
	{
		boost::trim(s.first);
	}

	replace_str = new char[max_length + 1];
	memset(replace_str, '*', max_length + 1);
}

void ChatFilter::filter(string& str)
{
	for (int i=0; i<dirty_word.size(); i++)
	{
		string& dirty = dirty_word[i].first;
		int len = dirty_word[i].second;

		replace_str[len] = NULL;
		boost::replace_all(str, dirty, replace_str);
		replace_str[len] = '*';
	} 
}

bool ChatFilter::has_dirty_word(const string& str)
{
	for (int i = 0; i < dirty_word.size(); i++)
	{
		// 욕설이 들어감
		if (str.find(dirty_word[i].first) != string::npos)
			return true;
	}

	return false;
}
