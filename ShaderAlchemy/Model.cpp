#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <execution>
#include <sstream>
#include <stb_image.h>
#include "JinGL/TextureLoader.h"
#include "JinGL/Buffer.h"
#include <set>

#include <meshoptimizer.h>
#include <glm/ext/matrix_transform.hpp>

std::tuple<std::vector<MeshVertex>, std::vector<unsigned int>> Optimize(const float* vertices, const unsigned int* indices, size_t verticesCount, size_t indicesCount) {

	std::vector<unsigned int> remap(indicesCount);
	size_t vertex_count = meshopt_generateVertexRemap(&remap[0], indices, indicesCount, vertices, verticesCount, sizeof(MeshVertex));

	std::vector<unsigned int> optIndices(indicesCount);
	std::vector<MeshVertex> optVertices(vertex_count);
	meshopt_remapIndexBuffer(optIndices.data(), indices, indicesCount, &remap[0]);
	meshopt_remapVertexBuffer(optVertices.data(), &vertices[0], verticesCount, sizeof(MeshVertex), &remap[0]);

	meshopt_optimizeVertexCache(optIndices.data(), optIndices.data(), indicesCount, vertex_count);
	meshopt_optimizeOverdraw(optIndices.data(), optIndices.data(), indicesCount, &optVertices[0].position.x, vertex_count, sizeof(MeshVertex), 1.05f);

	return { std::move(optVertices), std::move(optIndices) };
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root, const glm::mat4& transform)
{
	std::vector<MeshVertex> vertices(mesh->mNumVertices);
	std::fill(vertices.begin(), vertices.end(), MeshVertex{
			{0,0,0},
			{0,0,0},
			{0,0,0},
			{0,0},
		});

	if (mesh->HasPositions())
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices[i].position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		}
	}

	if (mesh->HasNormals()) {
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices[i].normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		}
	}

	if (mesh->HasTangentsAndBitangents())
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices[i].tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
		}
	}

	if (mesh->HasTextureCoords(0))
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices[i].uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
	}

	std::vector<unsigned int> indices;
	indices.reserve(mesh->mNumFaces * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

#define OPTIMIZE

#ifdef OPTIMIZE
	printf("Optimized Version\n");
	auto [optVertices, optIndices] = Optimize((const float*)vertices.data(), indices.data(), vertices.size(), indices.size());

	auto verticesSize = sizeof(MeshVertex) * optVertices.size();
	auto indicesSize = sizeof(unsigned int) * optIndices.size();

	Geometry* geometry = new Geometry(optVertices.data(), verticesSize, optIndices.data(), indicesSize);

#else
	printf("UnOptimized Version\n");
	auto verticesSize = sizeof(MeshVertex) * vertices.size();
	auto indicesSize = sizeof(unsigned int) * indices.size();

	Geometry* geometry = new Geometry(vertices.data(), verticesSize, indices.data(), indicesSize);

#endif // OPTIMIZE

	Mesh gpuMesh = {
		.geometry = geometry,
		.bounds = {
			.min = {mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z},
			.max = {mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z},
		},
		.textures = {},
		.visible = true,
		.transform = transform
	};

	if (scene->HasMaterials())
	{
		auto mat = scene->mMaterials[mesh->mMaterialIndex];
		if (mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[0] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[0] = TextureLoader::Load(ss.str());
		}

		if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_SPECULAR, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[1] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_METALNESS) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_METALNESS, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[1] = TextureLoader::Load(ss.str());
		}

		if (mat->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[2] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_NORMALS, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[2] = TextureLoader::Load(ss.str());
		}

		if (mat->GetTextureCount(aiTextureType_SHININESS) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_SHININESS, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[3] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[3] = TextureLoader::Load(ss.str());
		}

		if (mat->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[4] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_LIGHTMAP) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_LIGHTMAP, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[4] = TextureLoader::Load(ss.str());
		}

		if (mat->GetTextureCount(aiTextureType_EMISSION_COLOR) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_EMISSION_COLOR, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[5] = TextureLoader::Load(ss.str());
		}
		else if (mat->GetTextureCount(aiTextureType_EMISSIVE) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_EMISSIVE, 0, &path);
			std::stringstream ss;
			ss << root << "\\" << path.C_Str();
			gpuMesh.textures[5] = TextureLoader::Load(ss.str());
		}
	}

	meshes.push_back(gpuMesh);
}

static inline glm::mat4 AssimpMat4ToGlmMat4(const aiMatrix4x4& from)
{
	glm::mat4 to{};
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const char* root, const glm::mat4& transform)
{
	glm::mat4 local_transform = AssimpMat4ToGlmMat4(node->mTransformation) * transform;

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, root, local_transform);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, root, local_transform);
	}
}

bool Model::Load(const char* root, const char* filename, float scale)
{
	Assimp::Importer importer;
	char fullPath[256];

	sprintf_s(fullPath, "%s\\%s", root, filename);

	const aiScene* scene = importer.ReadFile(fullPath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_ForceGenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_GenBoundingBoxes);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}

	auto transform = AssimpMat4ToGlmMat4(scene->mRootNode->mTransformation) * glm::scale(glm::mat4(1.0f), { scale, scale, scale });
	ProcessNode(scene->mRootNode, scene, root, transform);

	bounds.min = meshes[0].bounds.min;
	bounds.max = meshes[0].bounds.max;

	for (const auto& mesh : meshes) {
		bounds.min = glm::min(bounds.min, mesh.bounds.min);
		bounds.max = glm::max(bounds.max, mesh.bounds.max);
	}

	if (scene->HasMaterials())
	{
		TextureLoader::Get()->LoadPromisedTextures();
	}

	return true;
}

void Model::Destroy()
{
	std::set<Texture2D*> unique_textures;

	for (int i = 0; i < meshes.size(); i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (meshes[i].textures[j] != nullptr)
			{
				unique_textures.insert(meshes[i].textures[j]);
			}
		}

		delete meshes[i].geometry;
	}

	for (auto& t : unique_textures)
	{
		delete t;
	}

	meshes.clear();
}