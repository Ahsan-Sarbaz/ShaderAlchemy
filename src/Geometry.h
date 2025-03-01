#pragma once
#include "JinGL/Buffer.h"
#include "JinGL/VertexInput.h"

class Geometry
{
public:
	Geometry() = default;
	Geometry(void* vertices, size_t verticesSize, void* indices, size_t indicesSize, bool combine_buffers = true);
	~Geometry();

	void Bind(VertexInput* vertexInput, int stride);
	const Buffer* GetVertexBuffer() const { return m_vertex_buffer; }
	const Buffer* GetIndexBuffer() const { return m_index_buffer; }
	void* GetIndicesOffset() const { return m_indicesOffset; }
	unsigned int GetIndicesCount() const { return m_indicesCount; }

private:
	Buffer* m_vertex_buffer;
	Buffer* m_index_buffer;
	void* m_indicesOffset;
	unsigned int m_indicesCount;
};
