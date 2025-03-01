#include "Geometry.h"
#include "Model.h"

Geometry::Geometry(void* vertices, size_t verticesSize, void* indices, size_t indicesSize, bool combine_buffer)
{
    m_indicesCount = indicesSize / sizeof(unsigned int);

    if (combine_buffer)
    {
        m_indicesOffset = (void*)verticesSize;

        m_vertex_buffer = new Buffer(verticesSize + indicesSize, nullptr, true);
        m_vertex_buffer->SubData(verticesSize, 0, vertices);
        m_vertex_buffer->SubData(indicesSize, (size_t)m_indicesOffset, indices);
        m_index_buffer = m_vertex_buffer;
    }
    else
    {
        m_vertex_buffer = new Buffer(verticesSize, vertices, false);
        m_index_buffer = new Buffer(indicesSize, indices, false);
        m_indicesOffset = 0;
    }
}

Geometry::~Geometry()
{
    delete m_vertex_buffer;
    if (m_vertex_buffer != m_index_buffer)
    {
        delete m_index_buffer;
    }
}

void Geometry::Bind(VertexInput* vertexInput, int stride)
{
    vertexInput->SetIndexBuffer(*m_index_buffer);
    vertexInput->SetVertexBuffer(*m_vertex_buffer, 0, stride, 0);
}
