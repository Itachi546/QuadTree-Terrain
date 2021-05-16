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
	pipelineDesc.rasterizationState.enableDepthTest = true;
	pipelineDesc.rasterizationState.enableDepthWrite = false;
	pipelineDesc.rasterizationState.depthTestFunction = CompareOp::LessOrEqual;
 
	// Line Pipeline
	s_DebugData.line_pipeline = Device::create_pipeline(pipelineDesc);

	// No Depth Test
	pipelineDesc.rasterizationState.enableDepthTest = false;
	s_DebugData.line_pipeline_no_depth = Device::create_pipeline(pipelineDesc);

	// Point Pipeline No Depth Test
	pipelineDesc.rasterizationState.topology = Topology::Point;
	pipelineDesc.rasterizationState.pointSize = 1.0f;
	s_DebugData.point_pipeline_no_depth = Device::create_pipeline(pipelineDesc);

	// Point pipeline
	pipelineDesc.rasterizationState.enableDepthTest = true;
	s_DebugData.point_pipeline = Device::create_pipeline(pipelineDesc);

	// Shape Pipeline
	pipelineDesc.rasterizationState.enableDepthTest = false;
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
	initialize_box(context);
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
	uint32_t offset = s_DebugData.bufferOffset;
	DebugDrawData defaultData = { glm::mat4(1.0f), glm::vec3(1.0f) };

	if (s_DebugData.lines.size() + s_DebugData.boxes.size() > 0)
	{
		context->set_graphics_pipeline(s_DebugData.line_pipeline);
		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);

		uint32_t box_vertex_count = s_DebugData.box_vertex_count;
		uint32_t offset = s_DebugData.circleOffset;
		if (s_DebugData.boxes.size() > 0)
		{
			context->set_buffer(s_DebugData.buffer, offset);
			for (auto& box : s_DebugData.boxes)
			{
				context->set_uniform(ShaderStage::Vertex, 0, sizeof(DebugDrawData), &box);
				context->draw(box_vertex_count);
			}

			s_DebugData.boxes.clear();
		}

		// Draw with depth test ON
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(defaultData), &defaultData);
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
				context->set_line_width(float(width));
				context->draw(totalLines * 2);
				offset += lineDataSize;
			}
		}
		context->set_line_width(1.0f);
		s_DebugData.lines.clear();
	}

	uint32_t totalPoints = static_cast<uint32_t>(s_DebugData.points.size());
	if (totalPoints > 0)
	{
		context->copy(s_DebugData.buffer, s_DebugData.points.data(), offset, totalPoints * sizeof(Point));
		context->set_graphics_pipeline(s_DebugData.point_pipeline);
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(defaultData), &defaultData);

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

		offset += totalShapes * sizeof(Point);
		s_DebugData.shapes.clear();
	}


	// no depth test
	if (s_DebugData.no_depthlines.size() + s_DebugData.circles.size() > 0)
	{
		context->set_graphics_pipeline(s_DebugData.line_pipeline_no_depth);
		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(defaultData), &defaultData);

		uint32_t total_circles = static_cast<uint32_t>(s_DebugData.circles.size());
		context->set_buffer(s_DebugData.buffer, 0);
		if (total_circles > 0)
		{
			context->set_line_width(4.0f);
			uint32_t vertexCount = s_DebugData.circle_vertex_count;
			uint32_t sizeInByte = sizeof(DebugDrawData);
			for (auto& circle : s_DebugData.circles)
			{
				context->set_uniform(ShaderStage::Vertex, 0, sizeInByte, &circle);
				context->draw(vertexCount);
			}
			s_DebugData.circles.clear();
		}

		context->set_uniform(ShaderStage::Vertex, 0, sizeof(defaultData), &defaultData);
		for (auto& key_val : s_DebugData.no_depthlines)
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
		s_DebugData.no_depthlines.clear();
	}
	
	context->set_line_width(1.0f);
	totalPoints = static_cast<uint32_t>(s_DebugData.no_depthPoint.size());
	if (totalPoints > 0)
	{
		context->copy(s_DebugData.buffer, s_DebugData.no_depthPoint.data(), offset, totalPoints * sizeof(Point));
		context->set_graphics_pipeline(s_DebugData.point_pipeline_no_depth);
		context->set_uniform(ShaderStage::Vertex, 0, sizeof(defaultData), &defaultData);
		context->set_buffer(s_DebugData.buffer, offset);

		ShaderBindings* bindingArr[] = { globalBindings };
		context->set_shader_bindings(bindingArr, 1);
		context->draw(totalPoints);
		s_DebugData.no_depthPoint.clear();
	}

}

void DebugDraw::destroy()
{
	Device::destroy_buffer(s_DebugData.buffer);
	Device::destroy_pipeline(s_DebugData.line_pipeline);
	Device::destroy_pipeline(s_DebugData.line_pipeline_no_depth);
	Device::destroy_pipeline(s_DebugData.point_pipeline);
	Device::destroy_pipeline(s_DebugData.point_pipeline_no_depth);
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

	s_DebugData.circleOffset = size;
	s_DebugData.bufferOffset = size;
	s_DebugData.circle_vertex_count = static_cast<uint32_t>(points.size());
}

void DebugDraw::initialize_box(Context* context)
{
	glm::vec3 min = glm::vec3(-1.0f);
	glm::vec3 max = glm::vec3(1.0f);
	glm::vec3 color = glm::vec3(1.0f);
	const int width = 1;

	glm::vec3 v0 = min;
	glm::vec3 v1 = glm::vec3(max.x, min.y, min.z);
	glm::vec3 v2 = glm::vec3(max.x, min.y, max.z);
	glm::vec3 v3 = glm::vec3(min.x, min.y, max.z);

	glm::vec3 v4 = glm::vec3(min.x, max.y, min.z);
	glm::vec3 v5 = glm::vec3(max.x, max.y, min.z);
	glm::vec3 v6 = max;
	glm::vec3 v7 = glm::vec3(min.x, max.y, max.z);


	std::vector<Line> lines;
	lines.push_back({ {v0, color}, {v1, color} });
	lines.push_back({ {v1, color}, {v2, color} });
	lines.push_back({ {v2, color}, {v3, color} });
	lines.push_back({ {v3, color}, {v0, color} });

	lines.push_back({ {v4, color}, {v5, color} });
	lines.push_back({ {v5, color}, {v6, color} });
	lines.push_back({ {v6, color}, {v7, color} });
	lines.push_back({ {v7, color}, {v4, color} });

	lines.push_back({ {v0, color}, {v4, color} });
	lines.push_back({ {v1, color}, {v5, color} });
	lines.push_back({ {v2, color}, {v6, color} });
	lines.push_back({ {v3, color}, {v7, color} });

	s_DebugData.box_vertex_count = static_cast<uint32_t>(lines.size()) * 2;
	uint32_t size = sizeof(Line) * static_cast<uint32_t>(lines.size());
	context->copy(s_DebugData.buffer, lines.data(), s_DebugData.circleOffset, size);
	s_DebugData.bufferOffset += size;
}
