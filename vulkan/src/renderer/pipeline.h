#pragma once

#include "graphics_enums.h"
#include <stdint.h>
#include <string>
#include <memory>
#include <core/base.h>
#include <vector>

struct ShaderDescription
{
	ShaderStage shaderStage;
	std::string code;
	uint32_t sizeInByte;
};

struct RasterizationState
{
	Topology topology = Topology::Triangle;
	FaceCulling faceCulling = FaceCulling::Back;
	PolygonMode polygonMode = PolygonMode::Fill;

	bool enableDepthTest = false;
	bool enableDepthWrite = true;
	CompareOp depthTestFunction = CompareOp::LessOrEqual;
	float lineWidth = 1.0f;
	float pointSize = 1.0f;

};

struct BlendState
{
	bool enable = false;
	BlendOp src = BlendOp::SrcAlpha;
	BlendOp dst = BlendOp::OneMinusSrcAlpha;
};

class RenderPass;
struct PipelineDescription
{
	int shaderStageCount = 0;
	ShaderDescription* shaderStages = nullptr;
	RasterizationState rasterizationState = {};
	RenderPass* renderPass = nullptr;
	BlendState blendState = {};
};

class Pipeline
{
public:

	virtual ~Pipeline(){}
protected:
};