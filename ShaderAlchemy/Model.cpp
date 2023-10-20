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

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root)
{
	std::vector<MeshVertex> vertices(mesh->mNumVertices);
	std::fill(vertices.begin(), vertices.end(), MeshVertex{
			{0,0,0},
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

	if (mesh->HasTangentsAndBitangents())
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			vertices[i].tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
			vertices[i].bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
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

	auto verticesSize = sizeof(MeshVertex) * vertices.size();
	auto indicesSize = sizeof(unsigned int) * indices.size();
	auto indicesOffset = verticesSize;

	auto buffer = new Buffer(verticesSize + indicesSize, nullptr, true);
	buffer->SubData(verticesSize, 0, vertices.data());
	buffer->SubData(indicesSize, indicesOffset, indices.data());

	Mesh gpuMesh = {
		.buffer = buffer,
		.indicesOffset = (void*)indicesOffset,
		.indicesCount = (unsigned int)indices.size(),
		.bounds = {
			.min = {mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z},
			.max = {mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z},
		},
		.textures = {},
		.visible = true
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

void Model::ProcessNode(aiNode* node, const aiScene* scene, const char* root)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, root);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene, root);
	}
}

bool Model::Load(const char* root, const char* filename)
{
	Assimp::Importer importer;
	char fullPath[256];

	sprintf(fullPath, "%s\\%s", root, filename);

	const aiScene* scene = importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_FlipUVs |
		aiProcess_ForceGenNormals | aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality | aiProcess_FixInfacingNormals |
		aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}

	ProcessNode(scene->mRootNode, scene, root);

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

		delete meshes[i].buffer;
	}

	for (auto& t : unique_textures)
	{
		delete t;
	}

	meshes.clear();
}
