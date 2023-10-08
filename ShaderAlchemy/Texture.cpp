#include "Texture.h"
#include <stb_image.h>
#include <gl/glew.h>

bool Texture::Init(const std::string& path)
{
	int w, h, c;
	auto data = stbi_load(path.c_str(), &w, &h, &c, 0);
	if (data == nullptr) return false;

	auto result = Init(w, h, c, data);

	stbi_image_free(data);
	return result;
}

bool Texture::Init(int width, int height, int channels, unsigned char* data)
{
	this->width = width;
	this->height = height;
	this->channels = channels;

	int internalFormat{ GL_RGBA8 };
	int pixelFormat{ GL_RGBA };

	switch (channels)
	{
	case 3:
		internalFormat = GL_RGB8;
		pixelFormat = GL_RGB;
		break;
	case 2:
		internalFormat = GL_RG8;
		pixelFormat = GL_RG;
		break;
	case 1:
		internalFormat = GL_R8;
		pixelFormat = GL_RED;
		break;

	}

	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glTextureStorage2D(id, 1, internalFormat, width, height);
	glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);

	return true;
}

void Texture::Bind(int unit)
{
	glBindTextureUnit(unit, id);
}

void Texture::Destroy()
{
	if (id > 0)
		glDeleteTextures(1, &id);
}
