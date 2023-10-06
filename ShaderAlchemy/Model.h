#pragma once
#include <glm/glm.hpp>
#include <vector>

struct MeshVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;
};

struct Mesh
{
	unsigned int buffer;
	void* indicesOffset;
	unsigned int indicesCount;
};

struct Model
{
	std::vector<Mesh> meshes;
	glm::mat4 transform;

	bool Load(const char* root, const char* filename);
};