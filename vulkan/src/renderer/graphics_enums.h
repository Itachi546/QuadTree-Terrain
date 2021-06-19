#pragma once

#include <stdint.h>

enum class BufferUsageHint : uint8_t
{
	StaticRead,
	StaticDraw,
	StreamDraw,
	StreamRead,
	StreamCopy,
	DynamicDraw,
	DynamicRead,
	DynamicCopy,
	Invalid
};

enum class BufferTarget : uint8_t
{
	Vertex,
	Index,
	Uniform,
	ShaderStorage,
	Invalid
};

enum class IndexType : uint8_t
{
	UnsignedInt,
	UnsignedShort,
};

enum class ShaderStage
{
	Vertex,
	Fragment,
	Compute,
	Geometry
};

enum class Format
{
	B8G8R8A8_Unorm,
	R8G8B8_Unorm,
	R8G8B8A8_Unorm,
	R32Float,
	R16Float,
	R32G32Float,
	R32G32B32Float,
	R32G32B32A32Float,
	D32Float,
	D16Unorm,
	Undefined
};

enum class Topology
{
	Triangle,
	TriangleStrip,
	Line,
	Point
};

enum class FaceCulling
{
	Front,
	Back,
	FrontAndBack,
	None
};

enum class PolygonMode
{
	Fill,
	Line,
	Point
};
enum class CompareOp
{
	Less,
	Greater,
	Equal,
	LessOrEqual,
	GreaterOrEqual
};

enum class TextureType
{
	Color2D,
	Color3D,
	DepthStencil
};

enum class TextureFilter
{
	Linear,
	Nearest
};

enum WrapMode
{
	Repeat,
	ClampToEdge,
	ClampToBorder
};

enum class DescriptorType
{
	UniformBuffer,
	ImageSampler
};

enum class BlendOp
{
	SrcAlpha,
	OneMinusSrcAlpha
};