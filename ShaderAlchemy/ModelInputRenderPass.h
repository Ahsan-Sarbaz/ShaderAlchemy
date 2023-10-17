#pragma once
#include "RenderPass.h"
#include "Model.h"
#include "JinGL/VertexInput.h"

class ModelInputRenderPass : public RenderPass {

public:
	virtual void Init() override;
	virtual void Draw() override;
	virtual void OnImGui() override;

	inline void SetModel(Model* model) { this->model = model; }
	inline void SetVertexInput(VertexInput* vertexInput) { this->vertexInput = vertexInput; }

private:
	Model* model;
	VertexInput* vertexInput;
	
	float cameraOffsetY;
	float cameraOffsetZ;
	bool open_channel_settings{ false };
	int selected_channel{ 0 };

};