#include "Shader.h"
#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>

void Shader::Init(const std::string& name)
{
	id = glCreateProgram();
	this->name = name;
}

void Shader::Shutdown()
{
	if (vId != 0)
		glDeleteShader(vId);
	if (fId != 0)
		glDeleteShader(fId);

	glDeleteProgram(id);
}

bool Shader::AttachVertexShader(const std::string& source)
{
	if (source.empty()) {
		printf("Vertex Shader source is empty\n");
		isValid = false;
		return false;
	}

	// TODO : should we move this
	vertexSource = source;

	if (vId != 0) {
		glDetachShader(id, vId);
		glDeleteShader(vId);
	}

	vId = glCreateShader(GL_VERTEX_SHADER);
	if (vId == 0) {
		printf("Failed to Create Vertex Shader\n");
		isValid = false;
		return false;
	}

	const char* src = source.c_str();
	glShaderSource(vId, 1, &src, 0);
	glCompileShader(vId);

	int status;
	glGetShaderiv(vId, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		int infoLogLen;
		glGetShaderiv(vId, GL_INFO_LOG_LENGTH, &infoLogLen);
		auto log = new char[infoLogLen + 1];
		glGetShaderInfoLog(vId, infoLogLen, 0, log);
		log[infoLogLen] = 0;
		// TODO : add a proper console for this
		printf("Vertex Shader Error : %s\n", log);
		delete[] log;
		isValid = false;
		return false;
	}

	glAttachShader(id, vId);

	return true;
}

bool Shader::AttachFragmentShader(const std::string& source)
{
	if (source.empty()) {
		printf("Fragment Shader source is empty\n");
		isValid = false;
		return false;
	}

	// TODO : should we move this
	fragmentSource = source;

	if (fId != 0) {
		glDetachShader(id, fId);
		glDeleteShader(fId);
	}


	fId = glCreateShader(GL_FRAGMENT_SHADER);
	if (fId == 0) {
		printf("Failed to Create Fragment Shader\n");
		isValid = false;
		return false;
	}

	const char* src = source.c_str();
	glShaderSource(fId, 1, &src, 0);
	glCompileShader(fId);

	int status;
	glGetShaderiv(fId, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		int infoLogLen;
		glGetShaderiv(fId, GL_INFO_LOG_LENGTH, &infoLogLen);
		auto log = new char[infoLogLen + 1];
		glGetShaderInfoLog(fId, infoLogLen, 0, log);
		log[infoLogLen] = 0;
		// TODO : add a proper console for this
		printf("Fragment Shader Error : %s\n", log);
		delete[] log;
		isValid = false;
		return false;
	}

	glAttachShader(id, fId);

	return true;
}

bool Shader::CompileShader()
{
	if (vId == ~0ull || fId == ~0ull) {
		printf("Please Attach Vertex and Fragment Shader First\n");
		isValid = false;
		return false;
	}

	glLinkProgram(id);

	int status;
	glGetProgramiv(id, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		int infoLogLen;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);
		auto log = new char[infoLogLen + 1];
		glGetProgramInfoLog(id, infoLogLen, 0, log);
		log[infoLogLen] = 0;
		// TODO : add a proper console for this
		printf("Program Error : %s\n", log);
		delete[] log;
		isValid = false;
		return false;
	}

	isValid = true;

	return true;
}

void Shader::GetShaderUniformsInfo()
{
	if (isValid) {
		uniforms.clear();
		int count = 0;
		glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);
		for (int i = 0; i < count; ++i)
		{
			char name[255];
			int size;
			unsigned int type;
			glGetActiveUniform(id, i, 255, 0, &size, &type, name);
			auto location = glGetUniformLocation(id, name);
			ShaderUniform uniform = {
				.location = location,
				.name = std::string(name),
				.type = ShaderUniformType(type),
				.size = size
			};

			uniforms.push_back(uniform);
		}
	}
}

void Shader::Bind()
{
	glUseProgram(id);
}

int Shader::GetUniformLocation(const char* name)
{
	for (const auto& uniform : uniforms) {
		if (uniform.name == std::string(name))
		{
			return uniform.location;
		}
	}

	return -1;
}

void Shader::UniformInt(const char* name, int value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform1i(id, location, value);
	}
}

void Shader::UniformFloat(const char* name, float value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform1f(id, location, value);
	}
}

void Shader::UniformVec2(const char* name, const glm::vec2& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform2fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformVec3(const char* name, const glm::vec3& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform3fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformVec4(const char* name, const glm::vec4& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform4fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformMat2(const char* name, const glm::mat2& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix2fv(id, location, 1, false, glm::value_ptr(value));
	}
}

void Shader::UniformMat3(const char* name, const glm::mat3& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix3fv(id, location, 1, false, glm::value_ptr(value));
	}
}

void Shader::UniformMat4(const char* name, const glm::mat4& value)
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix4fv(id, location, 1, false, glm::value_ptr(value));
	}
}
