#pragma once

#include "vulkan_includes.h"
#include "renderer/graphics_enums.h"

#include "core/base.h"

class VkTypeConverter
{
public:
	static VkShaderStageFlagBits from(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case ShaderStage::Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		default:
			ASSERT_MSG(0, "Undefined ShaderType");
			return VkShaderStageFlagBits(0);
		}
		return VkShaderStageFlagBits(0);
	}

	static VkFormat from(Format format)
	{
		switch (format)
		{
		case Format::B8G8R8A8_Unorm:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::R8G8B8A8_Unorm:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::R8G8B8_Unorm: 
			return VK_FORMAT_R8G8B8_UNORM;
		case Format::R32Float:
			return VK_FORMAT_R32_SFLOAT;
		case Format::R16Float:
			return VK_FORMAT_R16_SFLOAT;
		case Format::R16G16B16A16Float:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R16G16B16Float:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case Format::R32G32Float:
			return VK_FORMAT_R32G32_SFLOAT;
		case Format::R32G32B32Float:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Format::R32G32B32A32Float:
			return VK_FORMAT_R32G32B32A32_SFLOAT;

		case Format::D32Float:
			return VK_FORMAT_D32_SFLOAT;
		case Format::D16Unorm:
			return VK_FORMAT_D16_UNORM;
		default:
			ASSERT_MSG(1, "Undefined Format");
			return VkFormat(0);
		}
		return VkFormat(0);
	}

	static Format to(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:
			return Format::B8G8R8A8_Unorm;
		case VK_FORMAT_R8G8B8_UNORM:
			return Format::R8G8B8_Unorm;
		case VK_FORMAT_R32_SFLOAT: 
			return Format::R32Float;
		case VK_FORMAT_R32G32_SFLOAT:
			return Format::R32G32Float;
		case VK_FORMAT_R32G32B32_SFLOAT:
			return Format::R32G32B32Float;
		case VK_FORMAT_D32_SFLOAT:
			return Format::D32Float;
		default:
			ASSERT_MSG(0, "Undefined Format");
			return Format::Undefined;
		}
		return Format::Undefined;
	}

	static VkIndexType from(IndexType type)
	{
		switch (type)
		{
		case IndexType::UnsignedInt:
			return VK_INDEX_TYPE_UINT32;
		case IndexType::UnsignedShort:
			return VK_INDEX_TYPE_UINT16;
		default:
			ASSERT_MSG(0, "Undefined IndexType");
		}
		return VkIndexType(0);
	}

	static VkPolygonMode from(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::Fill:
				return VK_POLYGON_MODE_FILL;
		case PolygonMode::Line:
			return VK_POLYGON_MODE_LINE;
		case PolygonMode::Point:
			return VK_POLYGON_MODE_POINT;
		default:
			ASSERT_MSG(0, "Undefined PolygonMode");
			return VkPolygonMode(0);
		}
		return VkPolygonMode(0);
	}

	static VkSamplerAddressMode from(WrapMode mode)
	{
		switch (mode)
		{
		case WrapMode::Repeat:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case WrapMode::ClampToEdge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case WrapMode::ClampToBorder:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		default:
			ASSERT_MSG(0, "Undefined Wrap Mode");
		}
		return VkSamplerAddressMode(0);
	}

	static VkFilter from(TextureFilter filter)
	{
		switch (filter)
		{
		case TextureFilter::Linear:
			return VK_FILTER_LINEAR;
		case TextureFilter::Nearest:
			return VK_FILTER_NEAREST;
		default:
			ASSERT_MSG(0, "Undefined TextureFilter");
		}
		return VkFilter(0);
	}

	static VkPrimitiveTopology from(Topology topology)
	{
		switch (topology)
		{
		case Topology::Triangle:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Topology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case Topology::Line:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Topology::Point:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		default:
			ASSERT_MSG(0, "Undefined Topology");
			return VkPrimitiveTopology(0);
		}
		return VkPrimitiveTopology(0);
	}

	static VkCullModeFlags from(FaceCulling cullMode)
	{
		switch (cullMode)
		{
		case FaceCulling::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case FaceCulling::Back:
			return VK_CULL_MODE_BACK_BIT;
		case FaceCulling::FrontAndBack:
			return VK_CULL_MODE_FRONT_AND_BACK;
		case FaceCulling::None:
			return VK_CULL_MODE_NONE;
		default:
			ASSERT_MSG(0, "Undefined CullMode");
			return VkCullModeFlags(0);
		}
		return VkCullModeFlags(0);
	}

	static VkCompareOp from(CompareOp op)
	{
		switch (op)
		{
		case CompareOp::Less:
			return VK_COMPARE_OP_LESS;
		case CompareOp::Greater:
			return VK_COMPARE_OP_GREATER;
		case CompareOp::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOp::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareOp::Equal:
			return VK_COMPARE_OP_EQUAL;
		default:
			ASSERT_MSG(0, "Undefined Compare Operation");
			return VkCompareOp(0);
		}
		return VkCompareOp(0);
	}

	static VkDescriptorType from(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::UniformBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		default:
			ASSERT_MSG(0, "Undefined Descriptor Type");
			return VkDescriptorType(0);
		}
		return VkDescriptorType(0);
	}

	static VkMemoryPropertyFlags from(BufferUsageHint usage, VkBufferUsageFlags& usageFlags)
	{
		switch (usage)
		{
		case BufferUsageHint::StaticRead:
		case BufferUsageHint::StaticDraw:
			usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		case BufferUsageHint::StreamDraw:
		case BufferUsageHint::StreamRead:
		case BufferUsageHint::StreamCopy:
		case BufferUsageHint::DynamicDraw:
		case BufferUsageHint::DynamicRead:
		case BufferUsageHint::DynamicCopy:
		case BufferUsageHint::Invalid:
			return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		default:
			ASSERT_MSG(0, "Undefined BufferUsageHint");
			return VkMemoryPropertyFlags(0);
		}
		return VkMemoryPropertyFlags(0);
	}
};



