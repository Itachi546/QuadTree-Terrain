#pragma once

#include "graphics_enums.h"

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