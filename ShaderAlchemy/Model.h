#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "JinGL/Texture2D.h"
#include "Geometry.h"

struct MeshVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

struct Mesh
{
	Geometry* geometry;
	AABB bounds;
	Texture2D* textures[6];
	bool visible;
	glm::mat4 transform;
};

struct aiMesh;
struct aiScene;
struct aiNode;

struct Model
{
	std::vector<Mesh> meshes;
	AABB bounds;

	bool Load(const char* root, const char* filename, float scale = 1.0f);

	void Destroy();

	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root, const glm::mat4& transform);
	void ProcessNode(aiNode* node, const aiScene* scene, const char* root, const glm::mat4& transform);
};