#include "FullScreenRenderPass.h"
#include "application.h"

void FullScreenRenderPass::Init()
{
	RenderPass::Init();

	std::stringstream ss;
	ss << "FullScreenRenderPass_" << Application::Get()->GetPassCount() + 1;
	name = ss.str();

	auto fb = new Framebuffer;

	std::vector<FramebufferAttachment> attachments;
	attachments.push_back(FramebufferAttachment{
		.format = GL_RGBA8,
		.type = FramebufferAttachmentType::Color
		});

	auto width = Application::instance->preview_fb->width;
	auto height = Application::instance->preview_fb->height;

	fb->Init(width, height, attachments);
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

void FullScreenRenderPass::Draw()
{
	if (shader->IsValid())
	{
		output->ClearColorAttachments({ 0, 0, 0, 1.0f });
		output->Bind();
		shader->Bind();

		shader->UniformVec3("iResolution", glm::vec3{ float(output->width), float(output->height) , 0 });
		shader->UniformFloat("iTime", Application::instance->time);
		Application::instance->DrawFullScreenQuad();
	}
}

void FullScreenRenderPass::OnImGui()
{
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
			is_image_clicked = ImGui::ImageButton(buff, reinterpret_cast<void*>((unsigned long long)(channel->texture->id)), size, { 0, 1 }, { 1, 0 });
		}
		else if (channel && channel->type == ChannelType::RENDERPASS && channel->pass)
		{
			is_image_clicked = ImGui::ImageButton(buff,
				reinterpret_cast<void*>((unsigned long long)(
					channel->pass->GetOutput()->attachments[0].id)), size, { 0, 1 }, { 1, 0 });
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
					c->texture = new Texture;
					if (!c->texture->Init(item))
					{
						delete c->texture;
						delete c;
						continue;
					}
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
