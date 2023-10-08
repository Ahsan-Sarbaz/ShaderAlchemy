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

	GetShaderUniformsInfo();
	return true;
}

void Shader::GetShaderUniformsInfo()
{
	if (isValid) {
		uniforms.clear();
		int count = 0;
		int size_in_bytes = 0;
		int offset = 0;
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

			int storage_size = 0;
			switch (uniform.type)
			{
			case ShaderUniformType::Float: storage_size = sizeof(float); break;
			case ShaderUniformType::Int: storage_size = sizeof(int); break;
			case ShaderUniformType::Vec2: storage_size = sizeof(float) * 2; break;
			case ShaderUniformType::Vec3: storage_size = sizeof(float) * 3; break;
			case ShaderUniformType::Vec4: storage_size = sizeof(float) * 4; break;
			case ShaderUniformType::Mat2: storage_size = sizeof(float) * 2 * 2; break;
			case ShaderUniformType::Mat3: storage_size = sizeof(float) * 3 * 3; break;
			case ShaderUniformType::Mat4: storage_size = sizeof(float) * 4 * 4; break;
			case ShaderUniformType::Sampler2D: storage_size = sizeof(void*); break;
			}

			size_in_bytes += size * storage_size;
			uniform.backup = (void*)offset;
			offset = size * storage_size;
			uniforms.push_back(uniform);
		}

		if (backup_buffer)
		{
			delete backup_buffer;
		}

		backup_buffer = new char[size_in_bytes];
		memset(backup_buffer, 0, size_in_bytes);
		for (int i = 0; i < count; ++i) {
			auto& u = uniforms[i];
			int storage_size = 0;
			u.backup = (void*)((size_t)backup_buffer + (size_t)u.backup);
		}
	}
}

void Shader::Bind() const
{
	glUseProgram(id);
}

int Shader::GetUniformLocation(const char* name) const
{
	for (const auto& uniform : uniforms) {
		if (uniform.name == std::string(name))
		{
			return uniform.location;
		}
	}

	return -1;
}

void Shader::UniformInt(const char* name, int value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform1i(id, location, value);
	}
}

void Shader::UniformFloat(const char* name, float value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform1f(id, location, value);
	}
}

void Shader::UniformVec2(const char* name, const glm::vec2& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform2fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformVec3(const char* name, const glm::vec3& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform3fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformVec4(const char* name, const glm::vec4& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniform4fv(id, location, 1, glm::value_ptr(value));
	}
}

void Shader::UniformMat2(const char* name, const glm::mat2& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix2fv(id, location, 1, false, glm::value_ptr(value));
	}
}

void Shader::UniformMat3(const char* name, const glm::mat3& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix3fv(id, location, 1, false, glm::value_ptr(value));
	}
}

void Shader::UniformMat4(const char* name, const glm::mat4& value) const
{
	auto location = GetUniformLocation(name);
	if (location >= 0)
	{
		glProgramUniformMatrix4fv(id, location, 1, false, glm::value_ptr(value));
	}
}

void Shader::UniformInt(int location, int value) const
{
	glProgramUniform1i(id, location, value);
}

void Shader::UniformFloat(int location, float value) const
{
	glProgramUniform1f(id, location, value);
}

void Shader::UniformVec2(int location, const glm::vec2& value) const
{
	glProgramUniform2fv(id, location, 1, glm::value_ptr(value));
}

void Shader::UniformVec3(int location, const glm::vec3& value) const
{
	glProgramUniform3fv(id, location, 1, glm::value_ptr(value));
}

void Shader::UniformVec4(int location, const glm::vec4& value) const
{
	glProgramUniform4fv(id, location, 1, glm::value_ptr(value));
}

void Shader::UniformMat2(int location, const glm::mat2& value) const
{
	glProgramUniformMatrix2fv(id, location, 1, false, glm::value_ptr(value));
}

void Shader::UniformMat3(int location, const glm::mat3& value) const
{
	glProgramUniformMatrix3fv(id, location, 1, false, glm::value_ptr(value));
}

void Shader::UniformMat4(int location, const glm::mat4& value) const
{
	glProgramUniformMatrix4fv(id, location, 1, false, glm::value_ptr(value));
}

void Shader::UniformVec2(int location, float* value) const
{
	glProgramUniform2fv(id, location, 1, value);
}

void Shader::UniformVec3(int location, float* value) const
{
	glProgramUniform3fv(id, location, 1, value);
}

void Shader::UniformVec4(int location, float* value) const
{
	glProgramUniform4fv(id, location, 1, value);
}

void Shader::UniformMat2(int location, float* value) const
{
	glProgramUniformMatrix2fv(id, location, 1, false, value);
}

void Shader::UniformMat3(int location, float* value) const
{
	glProgramUniformMatrix3fv(id, location, 1, false, value);
}

void Shader::UniformMat4(int location, float* value) const
{
	glProgramUniformMatrix4fv(id, location, 1, false, value);
}

void Shader::UpdateUniformFromBackup(size_t index)
{
	auto& u = uniforms[index];
	switch (u.type)
	{
	case ShaderUniformType::Float:	UniformFloat(u.location, ((float*)u.backup)[0]); break;
	case ShaderUniformType::Int:	UniformInt(u.location, ((int*)u.backup)[0]); break;
	case ShaderUniformType::Vec2:	UniformVec2(u.location, ((float*)u.backup)); break;
	case ShaderUniformType::Vec3:	UniformVec3(u.location, ((float*)u.backup)); break;
	case ShaderUniformType::Vec4:	UniformVec4(u.location, ((float*)u.backup)); break;
	case ShaderUniformType::Mat2:	UniformMat2(u.location, ((float*)u.backup)); break;
	case ShaderUniformType::Mat3:	UniformMat3(u.location, ((float*)u.backup)); break;
	case ShaderUniformType::Mat4:	UniformMat4(u.location, ((float*)u.backup)); break;
//	case ShaderUniformType::Sampler2D: storage_size = sizeof(void*); break;
	}
}
