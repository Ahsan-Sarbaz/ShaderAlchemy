#include "FullScreenRenderPass.h"
#include "application.h"
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
	output->AddAttachment(Format::RGBA8);
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

		auto resolution = glm::vec3{ float(output->GetWidth()), float(output->GetHeight()) , 0 };
		shader->UniformVec3("iResolution", glm::value_ptr(resolution));
		shader->UniformFloat("iTime", Application::instance->time);
		Application::instance->DrawFullScreenQuad();
	}
}

void FullScreenRenderPass::OnImGui()
{
	auto& uniforms = shader->GetUniforms();

	ImGui::Columns(2);

	auto size = ImVec2{ 120, 120 };

	for (int i = 0; i < channels.size(); i++)
	{
		ImGui::Text("Channel %d", i);

		auto channel = channels[i];
		auto is_image_clicked = false;

		char buff[128];
		sprintf_s(buff, "%sChannel%d%d", name.c_str(), i, int(this));
		if (channel && channel->type == ChannelType::EXTERNAL_IMAGE && channel->texture)
		{
			is_image_clicked = ImGui::ImageButton(buff, reinterpret_cast<void*>((unsigned long long)
				(channel->texture->GetID())), size, { 0, 1 }, { 1, 0 });
		}
		else if (channel && channel->type == ChannelType::RENDERPASS && channel->pass)
		{
			auto id = channel->pass->GetOutput()->GetColorAttachments()[0]->GetID();
			is_image_clicked = ImGui::ImageButton(buff,
				reinterpret_cast<void*>((unsigned long long)(id)), size, { 0, 1 }, { 1, 0 });
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
		ImGui::OpenPopup("ChannelSettings");
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("ChannelSettings", &open_channel_settings, ImGuiWindowFlags_AlwaysAutoResize))
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
