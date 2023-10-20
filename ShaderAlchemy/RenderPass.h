#pragma once
#include "ShaderProgramSource.h"
#include "JinGL/Texture2D.h"
#include "JinGL/Framebuffer.h"
#include <array>

enum class ChannelType : int
{
	EXTERNAL_IMAGE,
	RENDERPASS
};

class RenderPass;

struct Channel
{
	ChannelType type;
	union
	{
		RenderPass* pass;
		Texture2D* texture;
	};
};

class RenderPass
{
	
public:
	virtual void Init();
	virtual void Draw() = 0;
	virtual void OnImGui() = 0;
	
	void SetShader(ShaderProgramSource* shader) { this->shader = shader; }

	void Resize(int width, int height);

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() { return name; }
	Framebuffer* GetOutput() { return output; }
	ShaderProgramSource* GetShader() { return shader; }
	
	void SetChannel(int index, Channel* channel);
	Channel* GetChannel(int index) { return channels[index]; }
	void BindChannels(int offset = 0);

protected:
	Framebuffer* output { nullptr };
	ShaderProgramSource* shader{ nullptr };
	std::string name;
	std::array<Channel*, 16> channels{};
};
