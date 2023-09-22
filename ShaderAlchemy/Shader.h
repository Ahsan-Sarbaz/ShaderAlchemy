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

	bool IsValid() { return isValid; }

	void Bind();

	int GetUniformLocation(const char* name);

	void UniformInt(const char* name, int value);
	void UniformFloat(const char* name, float value);
	void UniformVec2(const char* name, const glm::vec2& value);
	void UniformVec3(const char* name, const glm::vec3& value);
	void UniformVec4(const char* name, const glm::vec4& value);
	void UniformMat2(const char* name, const glm::mat2& value);
	void UniformMat3(const char* name, const glm::mat3& value);
	void UniformMat4(const char* name, const glm::mat4& value);

	const std::string& GetName() { return name; }
	const std::string& GetVertexSource() { return vertexSource; }
	const std::string& GetFragmentSource() { return fragmentSource; }

private:
	bool isValid {false };
	unsigned int id{ 0 };
	unsigned int vId{ 0 };
	unsigned int fId{ 0 };
	std::vector<ShaderUniform> uniforms;

	std::string name;
	std::string vertexSource;
	std::string fragmentSource;
};
