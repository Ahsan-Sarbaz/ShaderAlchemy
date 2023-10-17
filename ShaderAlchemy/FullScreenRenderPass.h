#pragma once
#include "RenderPass.h"

class FullScreenRenderPass : public RenderPass 
{
public:
	virtual void Init() override;
	virtual void Draw() override;
	virtual void OnImGui() override;

private:
	bool open_channel_settings { false };
	int selected_channel { 0 };
};

