#pragma once

#include "graphics_enums.h"
#include <stdint.h>

class VertexBuffer
{
public:
	virtual BufferUsageHint get_usage_hint() const = 0;
	virtual int get_size() const = 0;
	virtual ~VertexBuffer() {}
private:
};

class IndexBuffer
{
public:
	virtual BufferUsageHint get_usage_hint() const = 0;
	virtual int get_size() const = 0;
	virtual ~IndexBuffer() {}
private:
};

class UniformBuffer
{
public:
	virtual BufferUsageHint get_usage_hint() const = 0;
	virtual int get_size() const = 0;
	virtual ~UniformBuffer() {}
private:
};

class ShaderStorageBuffer
{
public:
	virtual BufferUsageHint get_usage_hint() const = 0;
	virtual int get_size() const = 0;
	virtual ~ShaderStorageBuffer() {}
private:
};

class IndirectBuffer
{
public:
	virtual BufferUsageHint get_usage_hint() const = 0;
	virtual int get_size() const = 0;
	virtual ~IndirectBuffer() {}
private:
};

struct DrawIndexedIndirectData
{
	uint32_t    indexCount;
	uint32_t    instanceCount;
	uint32_t    firstIndex;
	int32_t     vertexOffset;
	uint32_t    firstInstance;
};




struct VertexBufferView
{
	VertexBuffer* buffer;
	uint32_t size;
	uint32_t offset;
};

struct IndexBufferView
{
	IndexBuffer* buffer;
	uint32_t size;
	uint32_t offset;
};
