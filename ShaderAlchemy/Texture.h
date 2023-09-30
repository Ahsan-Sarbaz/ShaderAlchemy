#pragma once
#include <string>

struct Texture
{
	unsigned int id{};
	int width, height;
	int channels;

	bool Init(const std::string& path);
	void Bind(int unit);
	void Destroy();
};
