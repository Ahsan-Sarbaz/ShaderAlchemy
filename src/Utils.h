#pragma once
#include <filesystem>
#include <string>

bool read_entire_file(const std::filesystem::path& path, std::string& string);