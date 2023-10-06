#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

void processMesh(Model& model, aiMesh* mesh, const aiScene* scene, const char* root)
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
	};

	model.meshes.push_back(gpuMesh);
}

void processNode(Model& model, aiNode* node, const aiScene* scene, const char* root)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(model, mesh, scene, root);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(model, node->mChildren[i], scene, root);
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
		aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}

	processNode(*this, scene->mRootNode, scene, root);
	transform = glm::mat4(1.0f);
	return true;
}
