#include "VertexInput.h"
#include <GL/glew.h>

static int _current_offset = 0;
static int _current_index = 0;

void VertexInput::Init()
{
	glCreateVertexArrays(1, &id);
	_current_offset = 0;
	_current_index = 0;
}

void VertexInput::AddVec4()
{
	AddAttrib(0, _current_index++, 4, GL_FLOAT, false, sizeof(float) * _current_offset);
}

void VertexInput::AddVec3()
{
	AddAttrib(0, _current_index++, 3, GL_FLOAT, false, sizeof(float) * _current_offset);
}

void VertexInput::AddVec2()
{
	AddAttrib(0, _current_index++, 2, GL_FLOAT, false, sizeof(float) * _current_offset);
}

void VertexInput::AddAttrib(int binding, int index, int size, int type, bool normalized, int offset)
{
	glVertexArrayAttribBinding(id, index, binding);
	glVertexArrayAttribFormat(id, index, size, type, normalized, offset);
	glEnableVertexArrayAttrib(id, index);
	_current_offset += size;
}

void VertexInput::SetVertexBuffer(unsigned int vb, int binding, int stride, int offset)
{
	glVertexArrayVertexBuffer(id, binding, vb, offset, stride);
}

void VertexInput::SetIndexBuffer(unsigned int ib)
{
	glVertexArrayElementBuffer(id, ib);
}

void VertexInput::Bind()
{
	glBindVertexArray(id);
}


