#include "ModelInputRenderPass.h"
#include "application.h"
#include <glm/gtc/matrix_transform.hpp>

void ModelInputRenderPass::Init()
{
	RenderPass::Init();

	std::stringstream ss;
	ss << "ModelInputRenderPass_" << Application::Get()->GetPassCount() + 1;
	name = ss.str();

	auto fb = new Framebuffer;

	std::vector<FramebufferAttachment> attachments;
	attachments.push_back(FramebufferAttachment{
		.format = GL_RGBA8,
		.type = FramebufferAttachmentType::Color
		});

	attachments.push_back(FramebufferAttachment{
		.format = GL_DEPTH24_STENCIL8,
		.type = FramebufferAttachmentType::DepthStencil
		});

	auto width = Application::instance->preview_fb->width;
	auto height = Application::instance->preview_fb->height;

	fb->Init(width, height, attachments);
	output = fb;

	shader = new Shader;
	shader->Init("ModeInputBase");
	std::string source;
	if (read_entire_file("Shaders\\ModelInputVertex.glsl", source))
	{
		shader->AttachVertexShader(source);
	}

	if (read_entire_file("Shaders\\ModelInputFragment.glsl", source))
	{
		shader->AttachFragmentShader(source);
	}

	shader->CompileShader();
	shader->GetShaderUniformsInfo();

	model = new Model;

	if (!model->Load("Models\\FlightHelmet", "FlightHelmet.gltf"))
	{
		printf("Failed to load model\n");
	}

	vertexInput = &Application::instance->meshVertexInput;
}

void ModelInputRenderPass::Draw()
{
	if (shader->IsValid())
	{
		output->ClearAttachments({ 0, 0, 0, 1.0f }, 1.0f, 0);
		output->Bind();
		shader->Bind();

		glm::mat4 projectionMatrix = glm::perspectiveFov(glm::radians(90.0f),
			float(output->width), float(output->height), 0.1f, 1000.0f);
		glm::mat4 viewMatrix = glm::mat4(1.0f);

		viewMatrix = glm::translate(glm::mat4(1.0f), { 0,0,-1 });

		shader->UniformVec3("iResolution", glm::vec3{ float(output->width), float(output->height) , 0 });
		shader->UniformFloat("iTime", Application::instance->time);
		shader->UniformMat4("u_ProjectionMatrix", projectionMatrix);
		shader->UniformMat4("u_ViewMatrix", viewMatrix);
		shader->UniformMat4("u_ModelMatrix", model->transform);
		shader->Bind();

		glEnable(GL_DEPTH_TEST);
		vertexInput->Bind();
		for (const auto& mesh : model->meshes)
		{
			vertexInput->SetIndexBuffer(mesh.buffer);
			vertexInput->SetVertexBuffer(mesh.buffer, 0, sizeof(MeshVertex), 0);
			glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, mesh.indicesOffset);
		}
		glDisable(GL_DEPTH_TEST);
	}
}

void ModelInputRenderPass::OnImGui()
{
}
