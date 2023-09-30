#include "RenderPass.h"

void RenderPass::Init()
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		channels[i] = new Channel;
		channels[i]->type = ChannelType::EXTERNAL_IMAGE;
		channels[i]->texture = nullptr;
	}
}

void RenderPass::Resize(int width, int height) {
	if (output)
	{
		output->Resize(width, height);
	}
}

void RenderPass::SetChannel(int index, Channel* channel) {
	auto& c = channels[index];
	if (c != nullptr)
	{
		if (c->type == ChannelType::EXTERNAL_IMAGE && c->texture)
		{
			c->texture->Destroy();
			delete c->texture;
		}
		delete c;
	}
	c = channel;
}

void RenderPass::BindChannels(int offset) {
	for (size_t i = 0; i < channels.size(); i++)
	{
		auto& c = channels[i];
		if (c != nullptr) {
			if (c->type == ChannelType::EXTERNAL_IMAGE && c->texture)
			{
				c->texture->Bind(int(i) + offset);
			}
			else if (c->type == ChannelType::RENDERPASS && c->pass)
			{
				c->pass->GetOutput()->BindAttachment(0, int(i) + offset);
			}
		}
	}
}
