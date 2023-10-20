#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "JinGL/Buffer.h"
#include "JinGL/Texture2D.h"

struct MeshVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

struct Mesh
{
	Buffer* buffer;
	void* indicesOffset;
	unsigned int indicesCount;
	AABB bounds;
	Texture2D* textures[6];
	bool visible;
};

struct aiMesh;
struct aiScene;
struct aiNode;

struct Model
{
	std::vector<Mesh> meshes;
	AABB bounds;

	bool Load(const char* root, const char* filename);

	void Destroy();

	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root);
	void ProcessNode(aiNode* node, const aiScene* scene, const char* root);

};