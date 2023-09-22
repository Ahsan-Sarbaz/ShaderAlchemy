#pragma once
#include "Shader.h"
#include "Framebuffer.h"

class RenderPass
{
	
public:
	virtual void Init() = 0;
	virtual void Draw() = 0;
	
	void SetInput(RenderPass* input) { this->input = input; }
	void SetShader(Shader* shader) { this->shader = shader; }

	void Resize(int width, int height) {
		if (output)
		{
			output->Resize(width, height);
		}
	}

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() { return name; }
	Framebuffer* GetOutput() { return output; }
	Shader* GetShader() { return shader; }

protected:
	RenderPass* input { nullptr };
	Framebuffer* output { nullptr };
	Shader* shader{ nullptr };
	std::string name;
};
