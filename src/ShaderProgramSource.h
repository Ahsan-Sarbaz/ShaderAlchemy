#pragma once
#include <string>
#include "JinGL/Shader.h"

class ShaderProgramSource : public ShaderProgram
{
public:
	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() const { return name; }

	void SetVertexSource(const std::string& source) { this->vertex_source = source; }
	const std::string& GetVertexSource() { return vertex_source; }

	void SetFragmentSource(const std::string& source) { this->fragment_source = source; }
	const std::string& GetFragmentSource() { return fragment_source; }

private:
	std::string name;
	std::string vertex_source;
	std::string fragment_source;
};

