#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

static std::string ReplaceAll
(std::string str ,const std::string &from ,const std::string &to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from ,start_pos)) != std::string::npos)
	{	str.replace(start_pos ,from.length() ,to);
		start_pos += to.length();   }
	return str;
}

template<typename Out>
static void split(const std::string &s ,char delim ,Out result) {
	stringstream ss(s);
	std::string item;
	while (getline(ss ,item ,delim))
		*(result++) = item;
}

static std::vector<std::string> split(const std::string &s ,char delim) {
	std::vector<std::string> elems;
	split(s ,delim ,std::back_inserter(elems));
	return elems;
}
