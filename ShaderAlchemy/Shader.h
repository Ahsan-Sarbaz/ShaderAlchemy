#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

enum class ShaderUniformType : int {
	Float = 0x1406,
	Int = 0x1404,
	Vec2 = 0x8B50,
	Vec3 = 0x8B51,
	Vec4 = 0x8B52,
	Mat2 = 0x8B5A,
	Mat3 = 0x8B5B,
	Mat4 = 0x8B5C,
	Sampler2D = 0x8B5E,
};

struct ShaderUniform {
	int location;
	std::string name;
	ShaderUniformType type;
	int size;
	void* backup;
};

class Shader
{
public:
	void Init(const std::string& name);
	void Shutdown();

	bool AttachVertexShader(const std::string& source);
	bool AttachFragmentShader(const std::string& source);

	bool CompileShader();

	void GetShaderUniformsInfo();

	bool IsValid() const { return isValid; }

	void Bind() const;

	int GetUniformLocation(const char* name) const;

	void UniformInt(const char* name, int value) const;
	void UniformFloat(const char* name, float value)  const;
	void UniformVec2(const char* name, const glm::vec2& value) const;
	void UniformVec3(const char* name, const glm::vec3& value) const;
	void UniformVec4(const char* name, const glm::vec4& value) const;
	void UniformMat2(const char* name, const glm::mat2& value) const;
	void UniformMat3(const char* name, const glm::mat3& value) const;
	void UniformMat4(const char* name, const glm::mat4& value) const;

	void UniformInt(int location, int value) const;
	void UniformFloat(int location, float value)  const;
	void UniformVec2(int location, const glm::vec2& value) const;
	void UniformVec3(int location, const glm::vec3& value) const;
	void UniformVec4(int location, const glm::vec4& value) const;
	void UniformMat2(int location, const glm::mat2& value) const;
	void UniformMat3(int location, const glm::mat3& value) const;
	void UniformMat4(int location, const glm::mat4& value) const;

	void UniformVec2(int location, float* value) const;
	void UniformVec3(int location, float* value) const;
	void UniformVec4(int location, float* value) const;
	void UniformMat2(int location, float* value) const;
	void UniformMat3(int location, float* value) const;
	void UniformMat4(int location, float* value) const;

	void UpdateUniformFromBackup(size_t index);

	const std::string& GetName()  const { return name; }
	const std::string& GetVertexSource() const { return vertexSource; }
	const std::string& GetFragmentSource() const { return fragmentSource; }
	const std::vector<ShaderUniform> GetUniforms() const { return uniforms; }

private:
	bool isValid {false };
	unsigned int id{ 0 };
	unsigned int vId{ 0 };
	unsigned int fId{ 0 };
	std::vector<ShaderUniform> uniforms;

	std::string name;
	std::string vertexSource;
	std::string fragmentSource;

	char* backup_buffer;
};
