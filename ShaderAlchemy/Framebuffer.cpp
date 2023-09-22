#include "Framebuffer.h"
#include <GL/glew.h>

void Framebuffer::Init(int width, int height, const std::vector<FramebufferAttachment> attachments)
{
	this->width = width;
	this->height = height;
	this->attachments = attachments;

	glCreateFramebuffers(1, &id);

	int i = 0;
	for (auto& attachment : this->attachments)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &attachment.id);
		glTextureStorage2D(attachment.id, 1, attachment.format, width, height);
		glTextureParameteri(attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(id,
			attachment.type == FramebufferAttachmentType::Color ? (GL_COLOR_ATTACHMENT0 + i) : GL_DEPTH_STENCIL_ATTACHMENT, attachment.id, 0);
	}

	if (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer is not complete!\n");
	}

}

void Framebuffer::Shutdown()
{
	glDeleteFramebuffers(1, &id);
	for (int i = 0; i < (int)attachments.size(); ++i)
	{
		glDeleteTextures(1, &attachments[i].id);
	}

	attachments.clear();
}

void Framebuffer::Resize(int width, int height)
{
	this->width = width;
	this->height = height;

	for (int i = 0; i < (int)attachments.size(); ++i)
	{
		glDeleteTextures(1, &attachments[i].id);
	}

	int i = 0;
	for (auto& attachment : this->attachments)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &attachment.id);
		glTextureStorage2D(attachment.id, 1, attachment.format, width, height);
		glTextureParameteri(attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(id,
			attachment.type == FramebufferAttachmentType::Color ? (GL_COLOR_ATTACHMENT0 + i) : GL_DEPTH_STENCIL_ATTACHMENT, attachment.id, 0);
	}

	if (glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer is not complete!\n");
	}

}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glViewport(0, 0, width, height);
}

void Framebuffer::ClearColorAttachments(const glm::vec4& color)
{
	float clearColor[4] = { color.r, color.g, color.b, color.a };

	for (int i = 0; i < (int)attachments.size(); i++)
	{
		auto& attachment = attachments[i];
		if (attachment.type == FramebufferAttachmentType::Color)
		{
			glClearNamedFramebufferfv(id, GL_COLOR, i, clearColor);
		}
	}
}

void Framebuffer::ClearDepthStencilAttachment(float depth, int stencil)
{
	for (int i = 0; i < (int)attachments.size(); i++)
	{
		auto& attachment = attachments[i];
		if (attachment.type == FramebufferAttachmentType::DepthStencil)
		{
			glClearNamedFramebufferfi(id, GL_DEPTH_STENCIL, 0, depth, stencil);
		}
	}
}

void Framebuffer::ClearAttachments(const glm::vec4& color, float depth, int stencil)
{
	float clearColor[4] = { color.r, color.g, color.b, color.a };

	for (int i = 0; i < (int)attachments.size(); i++)
	{
		auto& attachment = attachments[i];
		if (attachment.type == FramebufferAttachmentType::Color)
		{
			glClearNamedFramebufferfv(id, GL_COLOR, i, clearColor);
		}
		else if (attachment.type == FramebufferAttachmentType::DepthStencil)
		{
			glClearNamedFramebufferfi(id, GL_DEPTH_STENCIL, 0, depth, stencil);
		}
	}
}

