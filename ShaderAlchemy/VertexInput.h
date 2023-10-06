#pragma once

// TODO : add multiple bindings support

class VertexInput
{
public:
	void Init();
	void AddVec4();
	void AddVec3();
	void AddVec2();
	void AddAttrib(int binding, int index, int size, int type,bool normalized, int offset);

	void SetVertexBuffer(unsigned int vb, int binding, int stride, int offset);
	void SetIndexBuffer(unsigned int ib);

	void Bind();

	unsigned int GetID() { return id; }

private:
	unsigned int id;
};

