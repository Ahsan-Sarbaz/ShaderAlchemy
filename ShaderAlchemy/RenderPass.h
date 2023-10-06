#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include <array>

enum class ChannelType : int
{
	EXTERNAL_IMAGE,
	RENDERPASS
};

class RenderPass;

// TODO: make this into something sane
struct Channel
{
	ChannelType type;
	union
	{
		RenderPass* pass;
		Texture* texture;
	};
};

class RenderPass
{
	
public:
	virtual void Init();
	virtual void Draw() = 0;
	virtual void OnImGui() = 0;
	
	void SetShader(Shader* shader) { this->shader = shader; }

	void Resize(int width, int height);

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() { return name; }
	Framebuffer* GetOutput() { return output; }
	Shader* GetShader() { return shader; }
	
	void SetChannel(int index, Channel* channel);
	Channel* GetChannel(int index) { return channels[index]; }
	void BindChannels(int offset = 0);

protected:
	Framebuffer* output { nullptr };
	Shader* shader{ nullptr };
	std::string name;
	std::array<Channel*, 16> channels{};
};
