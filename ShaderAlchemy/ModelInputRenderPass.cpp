#include "ModelInputRenderPass.h"
#include "Application.h"
#include "Utils.h"

#include <glm/gtc/matrix_transform.hpp>
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
	output->AddAttachment(Format::RGBA8, true);
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
			float(output->GetWidth()), float(output->GetHeight()), 0.1f, 10000.0f);
		glm::vec3 cameraPos = { 0, -cameraOffsetY, ((cameraOffsetZ - (cameraOffsetZ * 3.5f))) };
		auto viewMatrix = glm::lookAt(cameraPos, { 0, 0, 0 }, { 0, 1, 0 });

		viewMatrix = glm::translate(viewMatrix, cameraPosition);
		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.x), { 1, 0, 0});
		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.y), { 0, 1, 0});
		viewMatrix = glm::rotate(viewMatrix, glm::radians(cameraRotation.z), { 0, 0, 1});

		shader->UniformMat4("u_ProjectionMatrix", glm::value_ptr(projectionMatrix));
		shader->UniformMat4("u_ViewMatrix", glm::value_ptr(viewMatrix));

		auto translation = glm::translate(glm::mat4(1.0f), objectPosition);
		auto rotation = glm::mat4(1.0f);

		rotation = glm::rotate(rotation, glm::radians(objectRotation.x), { 1, 0, 0 });
		rotation = glm::rotate(rotation, glm::radians(objectRotation.y), { 0, 1, 0 });
		rotation = glm::rotate(rotation, glm::radians(objectRotation.z), { 0, 0, 1 });

		auto scale = glm::scale(glm::mat4(1.0f), objectScale);

		auto modelMatrix = translation * rotation * scale;

		shader->UniformMat4("u_ModelMatrix", glm::value_ptr(modelMatrix));

		auto app = Application::instance;

		float resolution[3] = { float(output->GetWidth()), float(output->GetHeight()) , 0.0f };
		float mouseInput[4] = { app->mouse_position.x, app->mouse_position.y,
			app->mouse_left_button, app->mouse_right_button };

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
		shader->UniformVec3("iEyePosition", glm::value_ptr(cameraPosition));

		shader->Bind();

		glEnable(GL_DEPTH_TEST);
		vertexInput->Bind();
		for (const auto& mesh : model->meshes)
		{
			if (!mesh.visible)
				continue;

			mesh.geometry->Bind(vertexInput, sizeof(MeshVertex));
			
			for (int i = 0; i < 5; i++)
			{
				if (mesh.textures[i] != nullptr)
				{
					mesh.textures[i]->Bind(i);
				}
			}

			glDrawElements(GL_TRIANGLES, mesh.geometry->GetIndicesCount(), GL_UNSIGNED_INT, mesh.geometry->GetIndicesOffset());
		}
		glDisable(GL_DEPTH_TEST);
	}
}

void ModelInputRenderPass::OnImGui()
{
	auto size = ImVec2{ 120, 120 };

	ImGui::SeparatorText("Camera");
	
	ImGui::PushID("Camera");
	ImGui::DragFloat3("Position", glm::value_ptr(cameraPosition), 0.1f);
	ImGui::DragFloat3("Rotation", glm::value_ptr(cameraRotation), 0.1f, -180, 180);
	ImGui::PopID();
	
	ImGui::SeparatorText("Object");
	
	ImGui::PushID("Object");
	ImGui::DragFloat3("Position", glm::value_ptr(objectPosition), 0.1f);
	ImGui::DragFloat3("Rotation", glm::value_ptr(objectRotation), 0.1f, -180, 180);
	ImGui::DragFloat3("Scale", glm::value_ptr(objectScale), 0.1);
	ImGui::PopID();

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

				cameraPosition = {0, 0, 0};
				cameraRotation = {0, 0, 0};
				objectPosition = {0, 0, 0};
				objectRotation = {0, 0, 0};
				objectScale = {1, 1, 1};

				break;
			}
		}

		Application::instance->drop_items.clear();
	}

	if (model)
	{
		ImGui::SeparatorText("Meshes");

		for (size_t i = 0; i < model->meshes.size(); i++)
		{
			if (ImGui::TreeNode((void*)(model + i), "Mesh %d", i + 1))
			{
				auto& mesh = model->meshes[i];

				ImGui::PushID((void*)(&mesh + i));
				ImGui::Checkbox("Visible", &mesh.visible);
				ImGui::PopID();

				const char* names[6] = {
					"Diffuse / Base Color",
					"Specular / Metallness",
					"Normals / Normals Camera",
					"Shininess / Diffuse Roughness",
					"Ligth Map / Ambient Occlusion",
					"Emissive / Emission Color"
				};

				for (int j = 0; j < 6; j++)
				{
					if (mesh.textures[j] != nullptr)
					{
						ImGui::Text(names[j]);
						auto id = mesh.textures[j]->GetID();
						ImGui::Image(reinterpret_cast<void*>((unsigned long long)(id)), size, { 0, 1 }, { 1, 0 });
					}
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::SeparatorText("Channels");

	{
		ImGui::Columns(2);

		auto size = ImVec2{ 120, 120 };

		for (int i = 0; i < channels.size(); i++)
		{
			if (i > 7) 
			{
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
					auto& [texture, is_draw] = channel->pass->GetOutput()->GetColorAttachments()[0];
					auto id = texture->GetID();
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



}
