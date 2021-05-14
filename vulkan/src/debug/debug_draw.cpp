#include "debug_draw.h"
#include "renderer/pipeline.h"
#include "renderer/device.h"
#include "renderer/context.h"
#include "common/common.h"
#include "renderer/buffer.h"
#include <algorithm>

DebugDraw::DebugData DebugDraw::s_DebugData = {};
void DebugDraw::init(Context* context)
{
	if (s_DebugData.initialized)
		return;

	PipelineDescription pipelineDesc = {};
	pipelineDesc.renderPass = context->get_global_renderpass();
	
	std::string vertexCode = load_file("spirv/line.vert.spv");
	std::string fragmentCode = load_file("spirv/line.frag.spv");

	ShaderDescription lineShaders[2] = {
		ShaderDescription{ShaderStage::Vertex,   vertexCode,   static_cast<uint32_t>(vertexCode.size())},
		ShaderDescription{ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size())},
	};
	pipelineDesc.shaderStageCount = 2;
	pipelineDesc.shaderStages = lineShaders;
	pipelineDesc.rasterizationState.faceCulling = FaceCulling::None;
	pipelineDesc.rasterizationState.topology = Topology::Line;
	pipelineDesc.rasterizationState.lineWidth = 1.0f;
	pipelineDesc.rasterizationState.enableDepthTest = false;
	pipelineDesc.rasterizationState.enableDepthWrite = false;
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
 
	// Line Pipeline
	s_DebugData.line_pipeline = Device::create_pipeline(pipelineDesc);

	// Point Pipeline
	pipelineDesc.rasterizationState.topology = Topology::Point;
	pipelineDesc.rasterizationState.pointSize = 1.0f;
	s_DebugData.point_pipeline = Device::create_pipeline(pipelineDesc);

	// Shape Pipeline
	pipelineDesc.rasterizationState.topology = Topology::Triangle;
	pipelineDesc.rasterizationState.pointSize = 1.0f;
	s_DebugData.shape_pipeline = Device::create_pipeline(pipelineDesc);

	// Fullscreen Quad
	vertexCode = load_file("spirv/image2D.vert.spv");
	fragmentCode = load_file("spirv/image2D.frag.spv");

	ShaderDescription quadShaders[2] = {
	ShaderDescription{ShaderStage::Vertex,   vertexCode,   static_cast<uint32_t>(vertexCode.size())},
	ShaderDescription{ShaderStage::Fragment, fragmentCode, static_cast<uint32_t>(fragmentCode.size())},
	};

	pipelineDesc.shaderStages = quadShaders;
	pipelineDesc.rasterizationState.enableDepthTest = false;

	pipelineDesc.blendState.enable = true;
	s_DebugData.fullscreenQuad = Device::create_pipeline(pipelineDesc);

	s_DebugData.buffer = Device::create_vertexbuffer(BufferUsageHint::StreamDraw, s_DebugData.totalSize);
	initialize_circle(context);
	s_DebugData.initialized = true;
}

void DebugDraw::immediate_draw_textured_quad(Context* context, ShaderBindings* bindings)
{
	context->set_graphics_pipeline(s_DebugData.fullscreenQuad);
	ShaderBindings* bindingArr[] = { bindings };
	context->set_shader_bindings(bindingArr, 1);
	context->draw(6);
}

void DebugDraw::render(Context* context, ShaderBindings* globalBindings)
{

	context->set_graphics_pipeline(s_DebugData.line_pipeline);

	uint32_t total_circles = static_cast<uint32_t>(s_DebugData.circles.size());
	if(total_circles > 0)
	{
		context->set_buffer(s_DebugData.buffer, 0);
		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);
		context->set_line_width(4.0f);
		uint32_t vertexCount = s_DebugData.circle_vertex_count;
		uint32_t sizeInByte = sizeof(glm::mat4);
		for (auto& circle : s_DebugData.circles)
		{
			context->set_uniform(ShaderStage::Vertex, 0, sizeInByte, &circle.model[0][0]);
			context->set_uniform(ShaderStage::Vertex, sizeInByte, sizeof(glm::vec3), &circle.color[0]);
			context->draw(vertexCount);
		}

		s_DebugData.circles.clear();
	}

	glm::mat4 model = glm::mat4(1.0f);
	glm::vec3 white = glm::vec3(1.0f);
	context->set_uniform(ShaderStage::Vertex, 0, sizeof(glm::mat4), &model[0][0]);
	context->set_uniform(ShaderStage::Vertex, sizeof(glm::mat4), sizeof(glm::vec3), &white[0]);

	uint32_t offset = s_DebugData.bufferOffset;
	for (auto& key_val : s_DebugData.lines)
	{
		auto& lines = key_val.second;
		uint32_t totalLines = static_cast<uint32_t>(lines.size());
		if (totalLines > 0)
		{
			auto width = key_val.first;
			uint32_t lineDataSize = sizeof(Line) * totalLines;
			context->copy(s_DebugData.buffer, lines.data(), offset, lineDataSize);
			context->set_buffer(s_DebugData.buffer, offset);
			ShaderBindings* bindingArr[] = { globalBindings };
			context->set_shader_bindings(bindingArr, 1);
			context->set_line_width(float(width));
			context->draw(totalLines * 2);
			offset += lineDataSize;
		}
	}
	context->set_line_width(1.0f);
	s_DebugData.lines.clear();

	uint32_t totalPoints = static_cast<uint32_t>(s_DebugData.points.size());
	if (totalPoints > 0)
	{
		context->copy(s_DebugData.buffer, s_DebugData.points.data(), offset, totalPoints * sizeof(Point));
		context->set_graphics_pipeline(s_DebugData.point_pipeline);
		context->set_buffer(s_DebugData.buffer, offset);
		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);
		context->draw(totalPoints);
		s_DebugData.points.clear();
		offset += totalPoints * sizeof(Point);
	}

	uint32_t totalShapes = static_cast<uint32_t>(s_DebugData.shapes.size());
	if (totalShapes > 0)
	{
		context->copy(s_DebugData.buffer, s_DebugData.shapes.data(), offset, totalShapes * sizeof(Point));
		context->set_graphics_pipeline(s_DebugData.shape_pipeline);
		context->set_buffer(s_DebugData.buffer, offset);
		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);
		context->draw(totalShapes);
		s_DebugData.shapes.clear();
	}

}

void DebugDraw::destroy()
{
	Device::destroy_buffer(s_DebugData.buffer);
	Device::destroy_pipeline(s_DebugData.line_pipeline);
	Device::destroy_pipeline(s_DebugData.point_pipeline);
	Device::destroy_pipeline(s_DebugData.shape_pipeline);
	Device::destroy_pipeline(s_DebugData.fullscreenQuad);
}

void DebugDraw::initialize_circle(Context* context)
{
	const uint32_t nPoints = 32;
	const float PI2 = float(PI * 2.0);
	const float stepSize = PI2 / nPoints;


	std::vector<Point> points;
	glm::vec2 p0 = glm::vec2{ cos(-stepSize), sin(-stepSize)};
	for (float theta = 0; theta <= PI2; theta += stepSize)
	{
		glm::vec2 p1 = glm::vec2{ cos(theta), sin(theta) };
		points.push_back(Point{ glm::vec3(p0, 0.0f), glm::vec3(1.0f) });
		points.push_back(Point{ glm::vec3(p1, 0.0f), glm::vec3(1.0f) });
		p0 = p1;
	};

	uint32_t size = static_cast<uint32_t>(points.size()) * sizeof(Point);
	context->copy(s_DebugData.buffer, points.data(), 0, size);
	s_DebugData.bufferOffset = size;

	s_DebugData.circle_vertex_count = static_cast<uint32_t>(points.size());
}
