#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <execution>
#include <stb_image.h>

int Model::PromiseTexture(const char* path)
{
	int index = (int)promisedTexturePaths.size();
	promisedTexturePaths.push_back(std::string(path));
	return index;
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const char* root)
{
	std::vector<MeshVertex> vertices(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices[i].position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		vertices[i].normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		vertices[i].tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
		vertices[i].bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
		vertices[i].uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
	}

	std::vector<unsigned int> indices;
	indices.reserve(mesh->mNumFaces * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	unsigned int buffer = 0;
	glCreateBuffers(1, &buffer);

	auto verticesSize = sizeof(MeshVertex) * vertices.size();
	auto indicesSize = sizeof(unsigned int) * indices.size();
	auto indicesOffset = verticesSize;

	glNamedBufferStorage(buffer, indicesSize + verticesSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(buffer, 0, verticesSize, vertices.data());
	glNamedBufferSubData(buffer, indicesOffset, indicesSize, indices.data());

	Mesh gpuMesh = {
		.buffer = buffer,
		.indicesOffset = (void*)indicesOffset,
		.indicesCount = (unsigned int)indices.size(),
		.bounds = {
			.min = {mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z},
			.max = {mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z},
		}
	};

	auto mat = scene->mMaterials[mesh->mMaterialIndex];
	if (mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
		gpuMesh.base_color_map = PromiseTexture(path.C_Str());
	}

	if (mat->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
	{
		aiString path;
		mat->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &path);
		gpuMesh.normal_map = PromiseTexture(path.C_Str());
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
	transform = glm::mat4(1.0f);

	bounds.min = meshes[0].bounds.min;
	bounds.max = meshes[0].bounds.max;

	for (const auto& mesh : meshes) {
		bounds.min = glm::min(bounds.min, mesh.bounds.min);
		bounds.max = glm::max(bounds.max, mesh.bounds.max);
	}

	struct PromisedTexture {
		int width, height, channels;
		unsigned char* data;
	};

	std::vector<PromisedTexture> promisedTextures;

	std::for_each(std::execution::par_unseq, promisedTexturePaths.begin(), promisedTexturePaths.end(),
		[&](const std::string& path) {
			
			char textureFullPath[256];
			sprintf(textureFullPath, "%s\\%s", root, path.c_str());
			PromisedTexture t = {};
			t.data = (unsigned char*)stbi_load(textureFullPath, &t.width, &t.height, &t.channels, 0);
			promisedTextures.push_back(t);
		});

	promisedTexturePaths.clear();

	for (const auto& t : promisedTextures)
	{
		auto new_texture = new Texture;
		if (!new_texture->Init(t.width, t.height, t.channels, t.data))
		{
			printf("Failed to Load Texture");
		}
		textures.push_back(new_texture);
	}
	
	return true;
}

void Model::Destroy()
{
	for (const auto& t : textures)
	{
		glDeleteTextures(1, &t->id);
	}

	for (int i = 0; i < meshes.size(); i++)
	{
		glDeleteBuffers(1, &meshes[i].buffer);
	}

	meshes.clear();
}
