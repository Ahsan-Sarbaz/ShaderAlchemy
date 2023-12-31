#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <string>
#include <filesystem>

#include "RenderPass.h"
#include "JinGL/VertexInput.h"
#include "JinGL/Framebuffer.h"
#include "ImGuiConsole.h"
#include "EditorPanel.h"

struct Application
{
	static Application* instance;

	GLFWwindow* window;
	glm::vec2 window_size;

	bool running{};
	bool playing{};

	float time{};
	float dt{};

	float frameRate;

	uint64_t frames{};
	uint64_t fps{};

	VertexInput* quadVertexInput;

	std::vector<EditorPanel*> editors;
	std::vector<RenderPass*> passes;
	std::vector<std::string> drop_items;

	RenderPass* selectedRenderPass{};

	Framebuffer* preview_fb;
	ShaderProgram* preview_shader;
	ImGuiConsole* console;
	
	bool mouse_left_button;
	bool mouse_right_button;
	glm::vec2 mouse_position;

	std::filesystem::path screenshot_output_directory = "Output\\ScreenShots\\";
	std::filesystem::path video_output_directory = "Output\\Videos\\";

	static Application* Get() { return instance; }

	void Init();
	void Run();
	void Shutdown();

	void CreateEditorPanel(const std::filesystem::path& path);
	void CreateEditorPanel(RenderPass* renderPass);
	void CreateFullScreenRenderPass();
	void CreateModelInputRenderPass();

	void InitQuadVoa();
	void DrawFullScreenQuad();

	void DrawAllPasses();

	void OnDrop(int count, const char* items[]);
	void OnWindowResize(int widht, int height);
	void OnPreviewResized(int width, int height);

	void OnRecord(int width, int height, int recording_time, int frame_rate, float speed);
	void OnTakeScreenShot(int width, int height);

	size_t GetPassCount() { return passes.size(); }

	static void Log(const char* fmt, ...);
};

