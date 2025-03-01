#include "Utils.h"
#include <fstream>

bool read_entire_file(const std::filesystem::path& path, std::string& string)
{
	std::ifstream file(path);

	if (!file.is_open()) return false;

	std::stringstream ss;
	ss << file.rdbuf();
	string = std::move(ss.str());
	return true;
}