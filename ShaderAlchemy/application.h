#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include "ImGuiTextColorEditor.h"

#include <string>
#include <filesystem>

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
// TODO: realy should this be here
	EditorPanelType type{};
	RenderPass* renderPass;
	int undoIndexOnDisk{0};
	void OnImGui();
};

struct Application
{

	static Application* instance;

	static Application* Get() { return instance; }

	GLFWwindow* window;
	glm::vec2 window_size;

	bool running{};
	bool playing{};

	float time{};
	float dt{};

	uint64_t frames{};
	uint64_t fps{};

	unsigned int quadVao{};

	std::vector<EditorPanel*> editors;
	std::vector<RenderPass*> passes;
	std::vector<std::string> drop_items;

	RenderPass* selectedRenderPass{};

	Framebuffer* preview_fb;
	Shader* preview_shader;

	std::filesystem::path screenshot_output_directory = "Output\\ScreenShots\\";
	std::filesystem::path video_output_directory = "Output\\Videos\\";

	void Init();
	void Run();
	void Shutdown();

	void CreateEditorPanel(const std::filesystem::path& path);
	void CreateEditorPanel(RenderPass* renderPass);
	void CreateShaderToyRenderPass();

	void InitQuadVoa();
	void DrawFullScreenQuad();

	void DrawAllPasses();

	void OnDrop(int count, const char* items[]);
	void OnWindowResize(int widht, int height);
	void OnPreviewResized(int width, int height);

	void OnRecord(int width, int height, int recording_time, int frame_rate, float speed);
	void OnTakeScreenShot(int width, int height);

	size_t GetPassCount() { return passes.size(); }
};

