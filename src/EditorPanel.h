#pragma once
#include "TextEditor.h"
#include "RenderPass.h"

enum class EditorPanelType
{
	VertexShader,
	FragmentShader
};

struct EditorPanel
{
	TextEditor* editor = nullptr;
	std::string name;
	EditorPanelType type{};
	RenderPass* renderPass;
	int undoIndexOnDisk{ 0 };

	void OnImGui();
};
