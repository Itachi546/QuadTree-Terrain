#pragma once

#include "core/math.h"
#include <vector>
#include <unordered_map>

class Pipeline;
class Context;
class ShaderBindings;
class VertexBuffer;

class DebugDraw
{
public:
	static void init(Context* context);
	// Always on top of everything 
	// So draw on default RenderPass
	static void draw_line(glm::vec3 start, glm::vec3 end, glm::vec3 color, uint32_t width = 1) 
	{
		s_DebugData.lines[width].push_back({ {start, color}, {end, color} });
		//s_DebugData.lines.push_back({ {start, color}, {end, color} });
	}

	static void draw_line_no_depth(glm::vec3 start, glm::vec3 end, glm::vec3 color, uint32_t width = 1)
	{
		s_DebugData.no_depthlines[width].push_back({ {start, color}, {end, color} });
	}

	static void draw_box(const glm::mat4& model, glm::vec3 color = glm::vec3(1.0f))
	{
		s_DebugData.boxes.push_back(DebugDrawData{model, color});
	}

	static void draw_points(glm::vec3 position, glm::vec3 color) 
	{
		s_DebugData.points.push_back({ position, color });
	}

	static void draw_points_no_depth(glm::vec3 position, glm::vec3 color)
	{
		s_DebugData.no_depthPoint.push_back({ position, color });
	}


	static void draw_triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
	{
		s_DebugData.shapes.push_back(Point{ v0, color });
		s_DebugData.shapes.push_back(Point{ v1, color });
		s_DebugData.shapes.push_back(Point{ v2, color });
	}

	static void draw_circle_no_depth(glm::mat4 model, glm::vec3 color)
	{
		s_DebugData.circles.push_back(DebugDrawData{ model, color });
	}

	static void immediate_draw_textured_quad(Context* context, ShaderBindings* bindings);

	static void render(Context* context, ShaderBindings* globalBinding);
	static void destroy();
private:
	static void initialize_circle(Context* context);
	static void initialize_box(Context* context);
	struct Point
	{
		glm::vec3 position;
		glm::vec3 color;
	};
	struct Line
	{
		Point p1;
		Point p2;
	};

	struct DebugDrawData
	{
		glm::mat4 model;
		glm::vec3 color;
	};

	struct DebugData
	{
		bool initialized = false;
		Pipeline* line_pipeline;
		Pipeline* line_pipeline_no_depth;

		std::unordered_map<uint32_t, std::vector<Line>> lines;
		std::unordered_map<uint32_t, std::vector<Line>> no_depthlines;

		Pipeline* point_pipeline;
		Pipeline* point_pipeline_no_depth;

		std::vector<Point> points;
		std::vector<Point> no_depthPoint;
		Pipeline* shape_pipeline;

		std::vector<Point> shapes;
		Pipeline* fullscreenQuad;

		std::vector<DebugDrawData> circles;
		uint32_t circle_vertex_count = 0;

		std::vector<DebugDrawData> boxes;
		uint32_t box_vertex_count = 0;

		VertexBuffer* buffer;
		const uint32_t totalSize = 1024 * 1024 * 10;
		uint32_t bufferOffset;
		uint32_t circleOffset;
	};
	static DebugData s_DebugData;
};