#pragma once
#include <string>

struct Texture
{
	unsigned int id{};
	int width, height;
	int channels;

	bool Init(const std::string& path);
	bool Init(int width, int height, int channels, unsigned char* data);
	void Bind(int unit);
	void Destroy();
};
