#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Texture.h"

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
	unsigned int buffer;
	void* indicesOffset;
	unsigned int indicesCount;
	AABB bounds;
	int base_color_map;
	int normal_map;
};

struct aiMesh;
struct aiScene;
struct aiNode;

struct Model
{
	std::vector<Mesh> meshes;
	std::vector<Texture*> textures;
	std::vector<std::string> promisedTexturePaths;
	glm::mat4 transform;
	AABB bounds;

	bool Load(const char* root, const char* filename);

	void Destroy();

	int PromiseTexture(const char* path);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root);
	void ProcessNode(aiNode* node, const aiScene* scene, const char* root);

};