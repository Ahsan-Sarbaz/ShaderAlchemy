#pragma once
#include <vector>
#include <glm/glm.hpp>

enum class FramebufferAttachmentType : unsigned char
{
	Color,
	DepthStencil
};

struct FramebufferAttachment
{
	unsigned int id = 0;
	int format = 0;
	FramebufferAttachmentType type{};
};

struct Framebuffer
{
	unsigned int id = 0;
	int width = 0;
	int height = 0;
	std::vector<FramebufferAttachment> attachments;

	void Init(int width, int height, const std::vector<FramebufferAttachment> attachments);
	void Shutdown();

	void Resize(int width, int height);

	void Bind();
	void BindAttachment(int index, int unit);

	void ClearColorAttachments(const glm::vec4& color);
	void ClearDepthStencilAttachment(float depth, int stencil);
	void ClearAttachments(const glm::vec4& color, float depth, int stencil);
};