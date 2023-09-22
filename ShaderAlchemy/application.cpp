#include "application.h"

#include <fstream>
#include <sstream>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb_image_write.h>
#include "FontAwesom6.h"
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <array>

Application* Application::instance = nullptr;

bool read_entire_file(const std::filesystem::path& path, std::string& string) {
	std::ifstream file (path);

	if (!file.is_open()) return false;

	std::stringstream ss;
	ss << file.rdbuf();
	string = std::move(ss.str());
	return true;
}

void gl_debug_message_callback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	const char* _source;
	const char* _type;
	const char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

	default:
		_source = "UNKNOWN";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

	default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

	default:
		_severity = "UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n",
		id, _type, _severity, _source, message);
}


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
		auto app = (Application*)glfwGetWindowUserPointer(window);
		app->window_size = { float(width), float(height) };
		app->resized = true;
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

	glDebugMessageCallback(gl_debug_message_callback, 0);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);

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
	float fontSize = 30.0f;
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

	InitQuadVoa();

	preview_fb = new Framebuffer;
	{
		std::vector<FramebufferAttachment> attachments;
		attachments.push_back(FramebufferAttachment{
				.format = GL_RGBA8,
				.type = FramebufferAttachmentType::Color
			});

		preview_fb->Init(int(window_size.x), int(window_size.y), attachments);
	}

	preview_shader = new Shader();
	{
		preview_shader->Init("Preview Shader");
		std::string source;
		if (read_entire_file("Shaders\\PreviewVertex.glsl", source))
			preview_shader->AttachVertexShader(source);

		if (read_entire_file("Shaders\\PreviewFragment.glsl", source))
			preview_shader->AttachFragmentShader(source);

		preview_shader->CompileShader();
	}

	if(!std::filesystem::exists(screenshot_output_directory))
		std::filesystem::create_directories(screenshot_output_directory);

	if (!std::filesystem::exists(video_output_directory))
		std::filesystem::create_directories(video_output_directory);

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

	int* cpu_side_buffer = nullptr;

	int ffmpeg_frames_to_write = 0;
	int ffmpeg_frame_counter = 0;

	FILE* ffmpeg = nullptr;
	recording = false;
	glm::vec2 last_preview_size = {};

	bool take_picture = false;

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

	bool want_exit = false;

	while (!glfwWindowShouldClose(window) && running) 
	{
		auto this_frame_time = (float)glfwGetTime();
		dt = this_frame_time - last_frame_time;

		if (playing || recording)
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

		// TODO: have a mode where we can only see the output of the selected pass and its input pass and so on
		// that way we can see per pass progress and also for performance reasons too

		if (take_picture || recording)
		{
			last_preview_size = preview_size;
			preview_size = selected_resolution;
			preview_resized = true;

			int buffer_size = int(selected_resolution.x) * int(selected_resolution.y);
			cpu_side_buffer = new int[buffer_size];

			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

			if (take_picture)
			{
				DrawAllPasses();
				glReadPixels(0, 0, int(selected_resolution.x), int(selected_resolution.y), GL_RGBA, GL_UNSIGNED_BYTE, cpu_side_buffer);
				stbi_flip_vertically_on_write(1);

				std::stringstream name_ss;
				name_ss << screenshot_output_directory.string() << "Output-" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S-") <<
					int(selected_resolution.x) << "x" << int(selected_resolution.y) << "p" << ".png";
				auto name = name_ss.str();

				if (!stbi_write_png(name.c_str(), int(selected_resolution.x), int(selected_resolution.y), 4, cpu_side_buffer, 4 * int(selected_resolution.x)))
				{
					printf("Failed to write output.png\n");
				}

				take_picture = false;
			}

			if (recording)
			{
				ffmpeg_frames_to_write = ((recording_time_minutes * 60) + recording_time_seconds) * (selected_frame_rate);

				std::stringstream ss;
				ss << "ffmpeg_bin\\ffmpeg.exe -benchmark -hide_banner -an -r " << int(selected_frame_rate) << " -f rawvideo -pix_fmt rgba -s ";
				ss << " " << int(selected_resolution.x) << "x" << int(selected_resolution.y) << " ";
				ss << " -i - -c:v hevc_nvenc -y -pix_fmt yuv420p -vf vflip -preset losslesshp ";
				ss << "\"" << video_output_directory << "Output-" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S-") <<
					int(selected_resolution.x) << "x" << int(selected_resolution.y) << "p" << ".mp4\"";
				std::string cmd = ss.str();

				// open pipe to ffmpeg's stdin in binary write mode
				ffmpeg = _popen(cmd.c_str(), "wb");

				time = 0;

				glfwSwapInterval(0);

				for (int frame_index = 0; frame_index < ffmpeg_frames_to_write; ++frame_index) 
				{
					DrawAllPasses();
					glReadPixels(0, 0, int(selected_resolution.x), int(selected_resolution.y), GL_RGBA, GL_UNSIGNED_BYTE, cpu_side_buffer);
					_fwrite_nolock(cpu_side_buffer, sizeof(int) * buffer_size, 1, ffmpeg);
					// TODO : add a customizable speed here 
					time += 1.0f / float(selected_frame_rate);
				}

				_pclose(ffmpeg);
				recording = false;
				ffmpeg_frame_counter = 0;
				ffmpeg_frames_to_write = 0;
				frames = 0;
				time = 0;

				glfwSwapInterval(1);
			}

			preview_size = last_preview_size;
			preview_resized = true;
			delete[] cpu_side_buffer;
			cpu_side_buffer = nullptr;
		}

		else
		{
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
					if (ImGui::Button("Yes", ImVec2(button_size, 0))) {
						running = false;
					}

					ImGui::SameLine();
					ImGui::SetItemDefaultFocus();
					if (ImGui::Button("No", ImVec2(button_size, 0))) {
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
					ImGui::Combo("VideoResolution", &resolution_index, resolution_strings.data(), resolution_strings.size());
					ImGui::SameLine();
					ImGui::Combo("VideoFPS", &frame_rate_index, frame_rate_strings.data(), frame_rate_strings.size());

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

					if (frame_rate_index == frame_rate_strings.size() - 1)
					{
						ImGui::InputInt("FPS", &selected_frame_rate);
					}
					else
					{
						selected_frame_rate = frame_rates[frame_rate_index];
					}

					ImGui::Separator();

					float button_size = 120;
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (button_size) / 2);
					if (ImGui::Button("Record", ImVec2(button_size, 0))) 
					{
						recording = true;
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
					ImGui::Combo("Resolution", &resolution_index, resolution_strings.data(), resolution_strings.size());
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
						take_picture = true;
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

				if (preview_size.x != avail.x || preview_size.y != avail.y)
				{
					preview_resized = true;
				}

				preview_size = { avail.x, avail.y };

				if (!preview_resized)
				{
					ImGui::Image(reinterpret_cast<void*>((unsigned long long)preview_fb->attachments[0].id),
						{ preview_size.x, preview_size.y }, { 0, 1 }, { 1, 0 });
				}
			}
			ImGui::End();

			ImGui::PopStyleVar();

			if (ImGui::Begin("Pipeline")) 
			{
				for (size_t i = 0; i < passes.size(); i++)
				{
					const auto pass = passes[i];
					bool selected = selectedRenderPass == pass;
					if (ImGui::Selectable((pass->GetName() + (selected ? ("    " ICON_FA_PLAY) : (""))).c_str())) 
					{
						selectedRenderPass = pass;
					}
				}

				if (ImGui::BeginPopupContextWindow("PipelineContextMenu",
					ImGuiPopupFlags_MouseButtonRight |
					ImGuiPopupFlags_NoOpenOverExistingPopup |
					ImGuiPopupFlags_NoOpenOverItems))
				{
					if (ImGui::MenuItem("Create ShaderToy Pass", "CTRL+S+T"))
					{
						CreateShaderToyRenderPass();
						selectedRenderPass = passes.back();
						preview_resized = true; // this is a hack to resize the framebuffer of a new added pass
					}
					ImGui::EndPopup();
				}
			}
			ImGui::End();

			// Rendering
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			ImGui::Render();
			glClear(GL_COLOR_BUFFER_BIT);
			if (resized) 
			{
				glViewport(0, 0, (int)window_size.x, (int)window_size.y);
				resized = false;
			}
			ImGuiIO& io = ImGui::GetIO();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		} // if (recording)

		glfwSwapBuffers(window);
	}
}

void Application::Shutdown() 
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void EditorPanel::OnImGui()
{
	if (ImGui::Begin(name.c_str(), 0, undoIndexOnDisk != editor->GetUndoIndex() ? ImGuiWindowFlags_UnsavedDocument : 0)) {
		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			auto shader = renderPass->GetShader();

			if (type == EditorPanelType::VertexShader)
			{
				shader->AttachVertexShader(editor->GetText());
			}
			else if (type == EditorPanelType::FragmentShader)
			{
				shader->AttachFragmentShader(editor->GetText());
			}

			shader->CompileShader();
			shader->GetShaderUniformsInfo();

			undoIndexOnDisk = editor->GetUndoIndex();
		}
		editor->Render((name + "Editor").c_str());
	}
	ImGui::End();
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
		ep->editor->SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
		ep->editor->SetText(shader->GetVertexSource());
		ep->name = shader->GetName() + " [VS]";
		ep->renderPass = renderPass;
		ep->type = EditorPanelType::VertexShader;
		editors.push_back(ep);
	}

	{
		auto ep = new EditorPanel;
		ep->editor = new TextEditor;
		ep->editor->SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
		ep->editor->SetText(shader->GetFragmentSource());
		ep->name = shader->GetName() + " [FS]";
		ep->renderPass = renderPass;
		ep->type = EditorPanelType::FragmentShader;
		editors.push_back(ep);
	}

}

class ShaderToyRenderPass : public RenderPass {

public:
	virtual void Init() override
	{
		name = "ShaderToyRenderPass";

		auto fb = new Framebuffer;

		std::vector<FramebufferAttachment> attachments;
		attachments.push_back(FramebufferAttachment{
				.format = GL_RGBA8,
				.type = FramebufferAttachmentType::Color
			});

		auto window_size = Application::instance->window_size;

		fb->Init(int(window_size.x), int(window_size.y), attachments);
		output = fb;

		shader = new Shader;
		shader->Init("ShaderToyBase");
		std::string source;
		if (read_entire_file("Shaders\\ShaderToyBaseVertex.glsl", source))
		{
			shader->AttachVertexShader(source);
		}

		if (read_entire_file("Shaders\\ShaderToyBaseFragment.glsl", source))
		{
			shader->AttachFragmentShader(source);
		}

		shader->CompileShader();
		shader->GetShaderUniformsInfo();
	}

	virtual void Draw() override
	{
		if (shader->IsValid()) {
			output->ClearColorAttachments({0, 0, 0, 1.0f});
			output->Bind();
			shader->Bind();

			if (input)
			{
				auto input_fb = input->GetOutput();
				if (input_fb)
				{
					auto color_attachment0 = input_fb->attachments[0].id;
					glBindTextureUnit(0, color_attachment0);
					shader->UniformInt("iLastPass", 0);
				}
			}

			shader->UniformVec3("iResolution", glm::vec3{float(output->width), float(output->height) , 0});
			shader->UniformFloat("iTime", Application::instance->time);
			Application::instance->DrawFullScreenQuad();
		}
	}
};

void Application::CreateShaderToyRenderPass()
{
	RenderPass* lastPass = nullptr;

	if (passes.size() > 0)
	{
		lastPass = passes.back();
	}

	auto rp = new ShaderToyRenderPass;
	rp->Init();
	rp->SetInput(lastPass);
	passes.push_back(rp);

	// TODO: move this someplace sane
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

	glCreateVertexArrays(1, &quadVao);

	unsigned int vbo;
	glCreateBuffers(1, &vbo);

	glNamedBufferData(vbo, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(quadVao, 0, vbo, 0, sizeof(float) * 6);
	glVertexArrayAttribFormat(quadVao, 0, 4, GL_FLOAT, false, 0);
	glVertexArrayAttribBinding(quadVao, 0, 0);
	glEnableVertexArrayAttrib(quadVao, 0);

	glVertexArrayAttribFormat(quadVao, 1, 2, GL_FLOAT, false, sizeof(float) * 4);
	glVertexArrayAttribBinding(quadVao, 1, 0);
	glEnableVertexArrayAttrib(quadVao, 1);

}

void Application::DrawFullScreenQuad()
{
	glBindVertexArray(quadVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Application::DrawAllPasses()
{
	if (preview_resized) {

		for (size_t i = 0; i < passes.size(); i++)
		{
			const auto& pass = passes[i];
			pass->Resize(int(preview_size.x), int(preview_size.y));
		}

		preview_fb->Resize(int(preview_size.x), int(preview_size.y));
		preview_resized = false;
	}

	for (size_t i = 0; i < passes.size(); i++)
	{
		const auto& pass = passes[i];
		pass->Draw();
	}

	if (passes.size() == 1)
	{
		selectedRenderPass = passes.back();
	}

	if (selectedRenderPass && selectedRenderPass->GetOutput())
	{
		preview_fb->Bind();
		glBindTextureUnit(0, selectedRenderPass->GetOutput()->attachments[0].id);
		preview_shader->Bind();
		preview_shader->UniformInt("inputColorAttachment", 0);
		DrawFullScreenQuad();
	}
}

