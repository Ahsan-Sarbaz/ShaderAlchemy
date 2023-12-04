#include "Application.h"

#include "JinGL/Debug.h"

#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "FontAwesom6.h"

#include "FullScreenRenderPass.h"
#include "ModelInputRenderPass.h"
#include "Utils.h"

extern "C" {
	__declspec(dllexport) int NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

Application* Application::instance = nullptr;

void Application::Init() {

	if (instance == nullptr)
	{
		instance = this;
	}
	else
	{
		printf("Creation of Application Twice is not allowed\n");
		return;
	}

	if(glfwInit() != GLFW_TRUE) {
		printf("Failed to init glfw\n");
		return;
	}

	window_size = {2560, 1440};

	const char* glsl_version = "#version 450";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(
		(int)window_size.x, (int)window_size.y,
		"ShaderAlchemy",
		nullptr, nullptr
	);

	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		Application::instance->OnWindowResize(width, height);
	});

	glfwSetDropCallback(window, [](GLFWwindow* window, int path_count, const char* paths[]) {
		Application::instance->OnDrop(path_count, paths);
	});

	if (window == nullptr) {
		printf("Failed to create GLFWwindow\n");
		return;
	}

	glfwSwapInterval(1);
	glfwMakeContextCurrent(window);

	if(glewInit() != GLEW_OK) {
		printf("Failed to init GLEW\n");
		return;
	}

	EnableDebugMessages();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.IniFilename = nullptr;
	//io.FontAllowUserScaling = true;
	ImGui::StyleColorsDark();

	// TODO: make this be relative to the display size
	float fontSize = 24.0f;
	io.Fonts->AddFontFromFileTTF("Fonts\\FiraCode-VariableFont_wght.ttf", fontSize);

	float baseFontSize = fontSize; // 13.0f is the size of the default font. Change to the font size you use.
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF("Fonts\\" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	preview_fb = new Framebuffer(int(window_size.x), int(window_size.y));
	preview_fb->AddAttachment(Format::RGBA8, true);
	preview_fb->Resize(int(window_size.x), int(window_size.y));

	{
		preview_shader = new ShaderProgram;
		std::string source;
		if (read_entire_file("Shaders\\PreviewVertex.glsl", source))
		{
			auto vs = new Shader(ShaderType::Vertex, source);
			preview_shader->AttachShader(vs);
		}

		if (read_entire_file("Shaders\\PreviewFragment.glsl", source))
		{
			auto fs = new Shader(ShaderType::Fragment, source);
			preview_shader->AttachShader(fs);
		}

		preview_shader->Link(nullptr, nullptr);
	}

	InitQuadVoa();

	if(!std::filesystem::exists(screenshot_output_directory))
		std::filesystem::create_directories(screenshot_output_directory);

	if (!std::filesystem::exists(video_output_directory))
		std::filesystem::create_directories(video_output_directory);

	console = new ImGuiConsole();

	Run();
	Shutdown();
}

void Application::Run() {

	running = true;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	float last_frame_time = 0.0f;
	float secondsCounter = 0.0f;

	uint64_t fpsCounter = 0;

	bool take_picture_imgui = false;
	bool record_imgui = false;

	int resolution_index = 2;
	int frame_rate_index = 2;

	constexpr std::array<const char*, 7> resolution_strings = {
		"480p	SD",
		"720p	HD",
		"1080p	FHD",
		"1440p	QHD (2K)",
		"2160p	UHD (4K)",
		"4320p	FUHD (8K)",
		"Custom",
	};

	constexpr std::array<const char*, 4> frame_rate_strings = {
		"24",
		"30",
		"60",
		"Custom",
	};

	constexpr std::array<int, 3> frame_rates = { 24, 30, 60 };

	constexpr std::array<glm::ivec2, 6> resolutions = {
		glm::ivec2{640, 480},
		glm::ivec2{1280, 720},
		glm::ivec2{1920, 1080},
		glm::ivec2{2560, 1440},
		glm::ivec2{3840, 2160},
		glm::ivec2{7680, 4320}
	};

	static_assert (resolutions.size() == (resolution_strings.size() - 1));
	static_assert (frame_rates.size() == (frame_rate_strings.size() - 1));

	glm::ivec2 selected_resolution = resolutions[resolution_index];
	int selected_frame_rate = frame_rates[frame_rate_index];

	int recording_time_minutes = 0;
	int recording_time_seconds = 10;
	float recording_speed = 1.0f;

	bool want_exit = false;
	bool open_channel_settings = false;
	int selected_channel = 0;

	mouse_position = { 0, 0 };

	while (!glfwWindowShouldClose(window) && running)
	{
		auto this_frame_time = (float)glfwGetTime();
		dt = this_frame_time - last_frame_time;
		frameRate = 1.0f / dt;

		if (playing)
		{
			time += dt;
			frames++;
			fpsCounter++;
			secondsCounter += dt;
			if (secondsCounter > 1.0f)
			{
				fps = fpsCounter;
				fpsCounter = 0;
				secondsCounter = 0.0f;
			}
		}

		last_frame_time = this_frame_time;

		glfwPollEvents();


		{
			mouse_left_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			mouse_right_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			if (mouse_left_button)
			{
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				mouse_position = { float(x), float(y) };
			}
		}

		// TODO: have a mode where we can only see the output of the selected pass and its input pass and so on
		// that way we can see per pass progress and also for performance reasons too

		DrawAllPasses();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		auto viewport_ds = ImGui::DockSpaceOverViewport();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
					want_exit = true;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Preview"))
			{
				if (ImGui::MenuItem(playing ? ("Pause " ICON_FA_PAUSE) : ("Play " ICON_FA_PLAY)))
				{
					playing = !playing;
				}

				if (ImGui::MenuItem("Reset"))
				{
					time = 0;
				}

				if (ImGui::MenuItem("Record") && selectedRenderPass)
				{
					record_imgui = true;
				}

				if (ImGui::MenuItem("Snap Shot") && selectedRenderPass)
				{
					take_picture_imgui = true;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (want_exit)
		{
			ImGui::OpenPopup("Exit?");
			ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Exit?", &want_exit, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to exit?");
				ImGui::Separator();

				float button_size = 120;
				auto avail = ImGui::GetContentRegionAvail().x;
				auto x = (avail / 2) - ((ImGui::GetStyle().ItemSpacing.x + button_size + button_size) / 2);
				ImGui::SetCursorPosX(x);
				if (ImGui::Button("Yes", ImVec2(button_size, 0)))
				{
					running = false;
				}

				ImGui::SameLine();
				ImGui::SetItemDefaultFocus();
				if (ImGui::Button("No", ImVec2(button_size, 0)))
				{
					want_exit = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		if (record_imgui)
		{
			ImGui::OpenPopup("RecordDetails");
			ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("RecordDetails", &record_imgui, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextColored({ 1,0,0,1 }, "Higher Resolution and Higher FPS will take longer");
				ImGui::InputInt("Minutes", &recording_time_minutes);
				ImGui::SameLine();
				ImGui::InputInt("Seconds", &recording_time_seconds);
				ImGui::Separator();
				ImGui::Combo("VideoResolution", &resolution_index, resolution_strings.data(), (int)resolution_strings.size());
				ImGui::SameLine();
				ImGui::Combo("VideoFPS", &frame_rate_index, frame_rate_strings.data(), (int)frame_rate_strings.size());

				if (resolution_index == (int)resolution_strings.size() - 1)
				{
					ImGui::InputInt("Width", &selected_resolution.x);
					ImGui::SameLine();
					ImGui::InputInt("Height", &selected_resolution.y);
				}
				else
				{
					selected_resolution = resolutions[resolution_index];
				}

				if (frame_rate_index == (int)frame_rate_strings.size() - 1)
				{
					ImGui::InputInt("FPS", &selected_frame_rate);
				}
				else
				{
					selected_frame_rate = frame_rates[frame_rate_index];
				}

				ImGui::InputFloat("Video Speed", &recording_speed);
				ImGui::Separator();

				float button_size = 120;
				ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (button_size) / 2);
				if (ImGui::Button("Record", ImVec2(button_size, 0)))
				{
					OnRecord(selected_resolution.x, selected_resolution.y, 
						recording_time_minutes * 60 + recording_time_seconds, selected_frame_rate, recording_speed);
				}

				ImGui::EndPopup();
			}
		}

		if (take_picture_imgui)
		{
			ImGui::OpenPopup("SnapShotDetails");
			ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("SnapShotDetails", &take_picture_imgui, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Combo("Resolution", &resolution_index, resolution_strings.data(), (int)resolution_strings.size());
				if (resolution_index == resolution_strings.size() - 1)
				{
					ImGui::InputInt("Width", &selected_resolution.x);
					ImGui::SameLine();
					ImGui::InputInt("Height", &selected_resolution.y);
				}
				else
				{
					selected_resolution = resolutions[resolution_index];
				}

				ImGui::Separator();

				if (ImGui::Button("Take"))
				{
					OnTakeScreenShot(selected_resolution.x, selected_resolution.y);
				}
				ImGui::EndPopup();
			}
		}

		for (auto& ep : editors)
		{
			if (selectedRenderPass == ep->renderPass)
			{
				ImGui::SetNextWindowDockID(viewport_ds);
				ep->OnImGui();
			}
		}

		ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
		float height = ImGui::GetFrameHeight();

		if (ImGui::BeginViewportSideBar("##Status Bar", viewport, ImGuiDir_Down, height, window_flags))
		{
			if (ImGui::BeginMenuBar())
			{
				ImGui::Text(ICON_FA_CLOCK " %.2f        " ICON_FA_FILM " %u        FPS %u", time, frames, fps);
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

		if (ImGui::Begin("Preview"))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			auto avail = ImGui::GetContentRegionAvail();
			ImGui::SetCursorPosX((avail.x / 2.0f) - ((ImGui::GetFontSize() * 6) / 2));

			// TODO: make this changeable
			float seek_speed = 5.0f;

			if (ImGui::Button(ICON_FA_REPEAT))
			{
				time = 0;
				frames = 0;
				fps = 0;
				fpsCounter = 0;
				secondsCounter = 0;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_BACKWARD_FAST))
			{
				time -= dt * seek_speed;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_BACKWARD_STEP))
			{
				time -= dt;
			}

			ImGui::SameLine();
			if (ImGui::Button(playing ? (ICON_FA_PAUSE) : (ICON_FA_PLAY)))
			{
				playing = !playing;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_FORWARD_STEP))
			{
				time += dt;
			}

			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_FORWARD_FAST))
			{
				time += dt * seek_speed;
			}

			ImGui::PopStyleColor();

			avail = ImGui::GetContentRegionAvail();
			if (avail.x != preview_fb->GetWidth() || avail.y != preview_fb->GetHeight())
			{
				OnPreviewResized(int(avail.x), int(avail.y));
			}

			auto& [texture, is_draw] = preview_fb->GetColorAttachments()[0];
			auto id = texture->GetID();
			ImGui::Image(reinterpret_cast<void*>((unsigned long long)id), avail, { 0, 1 }, { 1, 0 });
			
		}
		ImGui::End();

		ImGui::PopStyleVar();

		if (ImGui::Begin("Pipeline"))
		{
			for (size_t i = 0; i < passes.size(); i++)
			{
				const auto pass = passes[i];
				bool selected = selectedRenderPass == pass;
				if (ImGui::Selectable((pass->GetName() + (selected ? ("    " ICON_FA_PLAY) : (""))).c_str(), selected))
				{
					selectedRenderPass = pass;
				}
			}

			if (ImGui::BeginPopupContextWindow("PipelineContextMenu",
				ImGuiPopupFlags_MouseButtonRight |
				ImGuiPopupFlags_NoOpenOverExistingPopup |
				ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create FullScreen Pass"))
				{
					CreateFullScreenRenderPass();
					selectedRenderPass = passes.back();
				}

				if (ImGui::MenuItem("Create Model Input Pass"))
				{
					CreateModelInputRenderPass();
					selectedRenderPass = passes.back();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		if (!drop_items.empty()) 
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
			{
				ImGui::SetDragDropPayload("dropped_files", 0, 0);
				ImGui::EndDragDropSource();
			}
		}

		if (ImGui::Begin("Pass Properties")) 
		{
			if(selectedRenderPass)
				selectedRenderPass->OnImGui();
		}

		ImGui::End();

		console->Draw("Console");

		// Rendering
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
	}
}

void Application::Shutdown() 
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::CreateEditorPanel(const std::filesystem::path& path)
{
	auto ep = new EditorPanel;
	ep->editor = new TextEditor;

	std::string buffer;
	read_entire_file(path, buffer);
	ep->editor->SetText(buffer);
	ep->name = path.filename().string();
	editors.push_back(ep);
}

void Application::CreateEditorPanel(RenderPass* renderPass)
{
	auto shader = renderPass->GetShader();

	{
		auto ep = new EditorPanel;
		ep->editor = new TextEditor;
		ep->editor->SetLanguageDefinition(TextEditor::LanguageDefinition::Glsl());
		ep->editor->SetText(shader->GetVertexSource());
		ep->name = shader->GetName() + " [VS]";
		ep->renderPass = renderPass;
		ep->type = EditorPanelType::VertexShader;
		editors.push_back(ep);
	}

	{
		auto ep = new EditorPanel;
		ep->editor = new TextEditor;
		ep->editor->SetLanguageDefinition(TextEditor::LanguageDefinition::Glsl());
		ep->editor->SetText(shader->GetFragmentSource());
		ep->name = shader->GetName() + " [FS]";
		ep->renderPass = renderPass;
		ep->type = EditorPanelType::FragmentShader;
		editors.push_back(ep);
	}

}

void Application::CreateFullScreenRenderPass()
{
	auto rp = new FullScreenRenderPass;
	rp->Init();
	passes.push_back(rp);
	CreateEditorPanel(rp);
}

void Application::CreateModelInputRenderPass()
{
	auto rp = new ModelInputRenderPass;
	rp->Init();
	passes.push_back(rp);
	CreateEditorPanel(rp);
}

void Application::InitQuadVoa()
{
	float quadVerts[] =
	{
		-1,  1,  0, 1, 0, 1,// 0
		-1, -1,  0, 1, 0, 0,// 1
		 1, -1,  0, 1, 1, 0,// 2

		-1,  1,  0, 1, 0, 1,// 0
		 1, -1,  0, 1, 1, 0,// 2
		 1,  1,  0, 1, 1, 1	// 3
	};

	quadVertexInput = new VertexInput();
	quadVertexInput->AddVec4();
	quadVertexInput->AddVec2();

	Buffer* buffer = new Buffer(sizeof(quadVerts), quadVerts, false);
	quadVertexInput->SetVertexBuffer(*buffer, 0, sizeof(float) * 6, 0);
}

void Application::DrawFullScreenQuad()
{
	quadVertexInput->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Application::DrawAllPasses()
{
	for (size_t i = 0; i < passes.size(); i++)
	{
		const auto& pass = passes[i];
		pass->BindChannels();
		pass->Draw();
	}

	if (passes.size() == 1)
	{
		selectedRenderPass = passes.back();
	}

	if (selectedRenderPass && selectedRenderPass->GetOutput())
	{
		preview_fb->Bind();
		auto& [texture, is_draw] = selectedRenderPass->GetOutput()->GetColorAttachments()[0];
		texture->Bind(0);
		preview_shader->Bind();
		DrawFullScreenQuad();
	}
}

void Application::OnDrop(int count, const char* items[])
{
	for (int i = 0; i < count; i++)
	{
		drop_items.push_back(std::string(items[i]));
	}
}

void Application::OnWindowResize(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Application::OnPreviewResized(int width, int height)
{
	for (size_t i = 0; i < passes.size(); i++)
	{
		passes[i]->Resize(width, height);
	}

	preview_fb->Resize(width, height);
}

void Application::OnRecord(int width, int height, int recording_time, int frame_rate, float speed)
{
	auto last_preview_size = glm::ivec2{ preview_fb->GetWidth(), preview_fb->GetHeight()};

	auto buffer_size = width * height;
	auto cpu_side_buffer = new int[buffer_size];

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	auto ffmpeg_frames_to_write = recording_time * frame_rate;

	std::stringstream ss;
	ss << "ffmpeg_bin\\ffmpeg.exe -benchmark -hide_banner -an -r " << frame_rate << " -f rawvideo -pix_fmt rgba -s ";
	ss << " " << width << "x" << height << " ";
	ss << " -i - -c:v hevc_nvenc -y -pix_fmt yuv420p -vf vflip -preset losslesshp ";
	ss << "\"" << video_output_directory << "Output-" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S-") <<
		width << "x" << height << "p" << ".mp4\"";
	auto cmd = ss.str();

	auto ffmpeg = _popen(cmd.c_str(), "wb");
	time = 0;
	glfwSwapInterval(0);

	OnPreviewResized(width, height);

	while (ffmpeg_frames_to_write > 0) 
	{
		DrawAllPasses();
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, cpu_side_buffer);
		_fwrite_nolock(cpu_side_buffer, sizeof(int) * buffer_size, 1, ffmpeg);
		time += (1.0f / float(frame_rate)) * speed;
		ffmpeg_frames_to_write--;
	}


	_pclose(ffmpeg);
	frames = 0;
	time = 0;
	delete[] cpu_side_buffer;

	OnPreviewResized(last_preview_size.x, last_preview_size.y);
	glfwSwapInterval(1);
}

void Application::OnTakeScreenShot(int width, int height)
{
	auto last_preview_size = glm::ivec2{ preview_fb->GetWidth(), preview_fb->GetHeight()};

	auto buffer_size = width * height;
	auto cpu_side_buffer = new int[buffer_size];

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	OnPreviewResized(width, height);

	DrawAllPasses();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, cpu_side_buffer);
	
	std::stringstream ss;
	ss << "ffmpeg_bin\\ffmpeg.exe -benchmark -hide_banner -r 1 -f rawvideo -pix_fmt rgba -s " << width << "x" << height 
		<< " -i -  -vf \"fps = 1, scale = "<< width << ":-1 : flags = lanczos, vflip\" -f image2pipe -vcodec png "
		<< "\"" << screenshot_output_directory.string() << "Output-" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S-") <<
		width << "x" << height << "p" << ".png\"";
	auto cmd = ss.str();
	auto ffmpeg = _popen(cmd.c_str(), "wb");
	_fwrite_nolock(cpu_side_buffer, sizeof(int) * buffer_size, 1, ffmpeg);
	_pclose(ffmpeg);

	delete[] cpu_side_buffer;
	OnPreviewResized(last_preview_size.x, last_preview_size.y);
}

void Application::Log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	instance->console->AddLog(fmt, args);
	va_end(args);
}
