#include "FullScreenRenderPass.h"
#include "Application.h"
#include "Utils.h"

#include <glm/gtc/type_ptr.hpp>

void FullScreenRenderPass::Init()
{
	RenderPass::Init();

	std::stringstream ss;
	ss << "FullScreenRenderPass_" << Application::Get()->GetPassCount() + 1;
	name = ss.str();

	auto width = Application::instance->preview_fb->GetWidth();
	auto height = Application::instance->preview_fb->GetHeight();

	output = new Framebuffer(width, height);
	output->AddAttachment(Format::RGBA8, true);
	output->Resize(width, height);

	{
		shader = new ShaderProgramSource;
		std::string source;
		if (read_entire_file("Shaders\\ShaderToyBaseVertex.glsl", source))
		{
			auto vs = new Shader(ShaderType::Vertex, source);
			shader->AttachShader(vs);
			shader->SetVertexSource(source);
		}

		if (read_entire_file("Shaders\\ShaderToyBaseFragment.glsl", source))
		{
			auto fs = new Shader(ShaderType::Fragment, source);
			shader->AttachShader(fs);
			shader->SetFragmentSource(source);
		}

		shader->Link(nullptr, nullptr);
		shader->SetName("Full Screen");
	}
}

void FullScreenRenderPass::Draw()
{
	if (shader->IsValid())
	{
		output->ClearAttachments();
		output->Bind();
		shader->Bind();

		auto app = Application::instance;

		float resolution[3] = { float(output->GetWidth()), float(output->GetHeight()) , 0.0f };
		float mouseInput[4] = { app->mouse_position.x, app->mouse_position.y,
			app->mouse_left_button ? 1.0f : 0.0f, app->mouse_right_button ? 1.0f : 0.0f };

		{
			float channelResolutions[16 * 3] = {};
			float channelTimes[16] = {};

			int i = 0;
			for (auto& c : channels)
			{
				if (c == nullptr || c->texture == nullptr) continue;
				if (c->type == ChannelType::EXTERNAL_IMAGE)
				{
					channelResolutions[i + 0] = (float)c->texture->GetWidth();
					channelResolutions[i + 1] = (float)c->texture->GetHeight();
					channelResolutions[i + 2] = 0.0f;
				}
				else if (c->type == ChannelType::RENDERPASS)
				{
					channelResolutions[i + 0] = (float)c->pass->GetOutput()->GetWidth();
					channelResolutions[i + 1] = (float)c->pass->GetOutput()->GetHeight();
					channelResolutions[i + 2] = 0.0f;
				}

				channelTimes[i] = app->time;
				i++;
			}

			shader->UniformVec3Array("iChannelResolution", 16, channelResolutions);
			shader->UniformVec3Array("iChannelTime", 16, channelTimes);
		}

		shader->UniformVec3("iResolution", resolution);
		shader->UniformFloat("iTime", app->time);
		shader->UniformFloat("iFrameRate", app->frameRate);
		shader->UniformInt("iFrame", int(app->frames));
		shader->UniformFloat("iTimeDelta", app->dt);
		shader->UniformVec4("iMouse", mouseInput);
		Application::instance->DrawFullScreenQuad();
	}
}

void FullScreenRenderPass::OnImGui()
{
	ImGui::SeparatorText("Channels");

	ImGui::Columns(2);

	auto size = ImVec2{ 120, 120 };

	for (int i = 0; i < channels.size(); i++)
	{
		ImGui::Text("Channel %d", i);

		auto channel = channels[i];
		auto is_image_clicked = false;

		char buff[128];
		sprintf_s(buff, "%sChannel%d%llu", name.c_str(), i, this);
		if (channel && channel->type == ChannelType::EXTERNAL_IMAGE && channel->texture)
		{
			is_image_clicked = ImGui::ImageButton(buff, ((ImTextureID)
				(channel->texture->GetID())), size, { 0, 1 }, { 1, 0 });
		}
		else if (channel && channel->type == ChannelType::RENDERPASS && channel->pass)
		{
			auto& [texture, is_draw] = channel->pass->GetOutput()->GetColorAttachments()[0];
			auto id = texture->GetID();
			is_image_clicked = ImGui::ImageButton(buff,
				((ImTextureID)(id)), size, { 0, 1 }, { 1, 0 });
		}
		else
		{
			is_image_clicked = ImGui::ImageButton(buff, 0, size);
		}

		if (is_image_clicked)
		{
			open_channel_settings = true;
			selected_channel = i;
		}

		if (ImGui::BeginDragDropTarget())
		{
			auto payload = ImGui::AcceptDragDropPayload("dropped_files");
			ImGui::EndDragDropTarget();
			
			for (const auto& item : Application::instance->drop_items)
			{
				if (item.ends_with(".png") || item.ends_with(".jpg") || item.ends_with(".jpeg"))
				{
					auto c = new Channel;
					c->texture = new Texture2D(item.c_str(), true);
					c->type = ChannelType::EXTERNAL_IMAGE;
					SetChannel(i, c);
					break;
				}
			}

			Application::instance->drop_items.clear();
		}

		ImGui::NextColumn();
	}

	ImGui::Columns(1);

	if (open_channel_settings) 
	{
		ImGui::OpenPopup("Channel Settings");
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Channel Settings", &open_channel_settings, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("%s Channel %d Settings", GetName().c_str(), selected_channel);

			auto c = GetChannel(selected_channel);

			ImGui::Combo("Channel Type", (int*)(&c->type), "ImageFile\0RenderPass\0");

			if (c->type == ChannelType::RENDERPASS) 
			{
				static int current_item = 0;

				if (ImGui::BeginCombo("Render Pass", c->pass ? c->pass->GetName().c_str() : "Select Render Pass"))
				{
					for (size_t i = 0; i < Application::instance->passes.size(); i++)
					{
						bool selected = c->pass == Application::instance->passes[i];
						if (ImGui::Selectable(Application::instance->passes[i]->GetName().c_str(), selected))
						{
							c->pass = Application::instance->passes[i];
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				c->pass = nullptr;
			}

			ImGui::EndPopup();
		}
	}
}
