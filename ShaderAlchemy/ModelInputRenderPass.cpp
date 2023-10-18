#include "ModelInputRenderPass.h"
#include "application.h"
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

void ModelInputRenderPass::Init()
{
	RenderPass::Init();

	std::stringstream ss;
	ss << "ModelInputRenderPass_" << Application::Get()->GetPassCount() + 1;
	name = ss.str();

	auto width = Application::instance->preview_fb->GetWidth();
	auto height = Application::instance->preview_fb->GetHeight();

	output = new Framebuffer(width, height);
	output->AddAttachment(Format::RGBA8);
	output->AddDepthStencil();
	output->Resize(width, height);

	{
		shader = new ShaderProgramSource;
		std::string source;
		if (read_entire_file("Shaders\\ModelInputVertex.glsl", source))
		{
			auto vs = new Shader(ShaderType::Vertex, source);
			shader->AttachShader(vs);
			shader->SetVertexSource(source);
		}

		if (read_entire_file("Shaders\\ModelInputFragment.glsl", source))
		{
			auto fs = new Shader(ShaderType::Fragment, source);
			shader->AttachShader(fs);
			shader->SetFragmentSource(source);
		}

		shader->Link(nullptr, nullptr);
		shader->SetName("Model Input");
	}

	vertexInput = new VertexInput();
	vertexInput->AddVec3();
	vertexInput->AddVec3();
	vertexInput->AddVec3();
	vertexInput->AddVec3();
	vertexInput->AddVec2();
}

void ModelInputRenderPass::Draw()
{
	if (model != nullptr && shader->IsValid())
	{
		output->ClearAttachments();
		output->Bind();
		shader->Bind();

		glm::mat4 projectionMatrix = glm::perspectiveFov(glm::radians(90.0f),
			float(output->GetWidth()), float(output->GetHeight()), 0.1f, 1000.0f);
		glm::vec3 cameraPos = { 0, -cameraOffsetY, ((cameraOffsetZ - (cameraOffsetZ * 3.5f))) };
		auto viewMatrix = glm::lookAt(cameraPos, { 0, 0, 0 }, { 0, 1, 0 });

		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.x), { 1, 0, 0});
		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.y), { 0, 1, 0});
		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.z), { 0, 0, 1});
		viewMatrix = glm::translate(viewMatrix, cameraPosition);

		auto resolution = glm::vec3{ float(output->GetWidth()), float(output->GetHeight()) , 0 };

		shader->UniformVec3("iResolution", glm::value_ptr(resolution));
		shader->UniformFloat("iTime", Application::instance->time);
		shader->UniformMat4("u_ProjectionMatrix", glm::value_ptr(projectionMatrix));
		shader->UniformMat4("u_ViewMatrix", glm::value_ptr(viewMatrix));

		auto modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(objectRotation.x), { 1, 0, 0 });
		modelMatrix = glm::rotate(modelMatrix, glm::radians(objectRotation.y), { 0, 1, 0 });
		modelMatrix = glm::rotate(modelMatrix, glm::radians(objectRotation.z), { 0, 0, 1 });
		modelMatrix = glm::translate(modelMatrix, objectPosition);

		shader->UniformMat4("u_ModelMatrix", glm::value_ptr(modelMatrix));
		shader->Bind();

		glEnable(GL_DEPTH_TEST);
		vertexInput->Bind();
		for (const auto& mesh : model->meshes)
		{
			vertexInput->SetIndexBuffer(*mesh.buffer);
			vertexInput->SetVertexBuffer(*mesh.buffer, 0, sizeof(MeshVertex), 0);
			if (mesh.base_color_map != nullptr)
			{
				mesh.base_color_map->Bind(0);
			}
			if (mesh.normal_map != nullptr)
			{
				mesh.normal_map->Bind(1);
			}

			glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, mesh.indicesOffset);
		}
		glDisable(GL_DEPTH_TEST);
	}
}

void ModelInputRenderPass::OnImGui()
{
	auto size = ImVec2{ 120, 120 };

	ImGui::SeparatorText("Camera");
	ImGui::DragFloat3("Position", glm::value_ptr(cameraPosition));
	ImGui::DragFloat3("Rotation", glm::value_ptr(cameraRotation));
	ImGui::SeparatorText("Object");
	ImGui::DragFloat3("Object Position", glm::value_ptr(objectPosition));
	ImGui::DragFloat3("Object Rotation", glm::value_ptr(objectRotation));

	ImGui::SeparatorText("Model Input");
	ImGui::Image(0, size);

	if (ImGui::BeginDragDropTarget())
	{
		auto payload = ImGui::AcceptDragDropPayload("dropped_files");
		ImGui::EndDragDropTarget();

		for (const auto& item : Application::instance->drop_items)
		{
			if (item.ends_with(".gltf") || item.ends_with(".fbx") || item.ends_with(".obj"))
			{
				auto m = new Model;

				std::filesystem::path full_path(item);
				auto root = full_path.parent_path().string();
				auto file_name = full_path.filename().string();

				if (!m->Load(root.c_str(), file_name.c_str()))
				{
					delete m;
					continue;
				}
				
				if (model) 
				{
					model->Destroy();
					delete model;
				}

				model = m;

				cameraOffsetY = abs(model->bounds.min.y) * 0.5f;
				cameraOffsetZ = abs(model->bounds.min.z) * 2.5f;

				break;
			}
		}

		Application::instance->drop_items.clear();
	}
	
	ImGui::SeparatorText("Channels");

	{
		ImGui::Columns(2);

		auto size = ImVec2{ 120, 120 };

		for (int i = 0; i < channels.size(); i++)
		{
			if (i <= 7) continue;

			ImGui::Text("Channel %d", i);

			auto channel = channels[i];
			auto is_image_clicked = false;

			char buff[128];
			sprintf_s(buff, "%sChannel%d%d", name.c_str(), i, int(this));
			if (channel && channel->type == ChannelType::EXTERNAL_IMAGE && channel->texture)
			{
				is_image_clicked = ImGui::ImageButton(buff, reinterpret_cast<void*>((unsigned long long)(channel->texture->GetID())), size, { 0, 1 }, { 1, 0 });
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
						c->texture = new Texture2D(item.c_str(), false);
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



}
