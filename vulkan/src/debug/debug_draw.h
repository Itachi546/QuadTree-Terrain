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
	static void draw_points(glm::vec3 position, glm::vec3 color) 
	{
		s_DebugData.points.push_back({ position, color });
	}

	static void draw_triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color)
	{
		s_DebugData.shapes.push_back(Point{ v0, color });
		s_DebugData.shapes.push_back(Point{ v1, color });
		s_DebugData.shapes.push_back(Point{ v2, color });
	}

	static void draw_circle(glm::mat4 model, glm::vec3 color)
	{
		s_DebugData.circles.push_back(CircleData{ model, color });
	}

	static void immediate_draw_textured_quad(Context* context, ShaderBindings* bindings);

	static void render(Context* context, ShaderBindings* globalBinding);
	static void destroy();
private:
	static void initialize_circle(Context* context);
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

	struct CircleData
	{
		glm::mat4 model;
		glm::vec3 color;
	};

	struct DebugData
	{
		bool initialized = false;
		Pipeline* line_pipeline;
		std::unordered_map<uint32_t , std::vector<Line>> lines;

		Pipeline* point_pipeline;
		std::vector<Point> points;

		Pipeline* shape_pipeline;
		std::vector<Point> shapes;

		Pipeline* fullscreenQuad;

		std::vector<CircleData> circles;
		uint32_t circle_vertex_count = 0;

		VertexBuffer* buffer;
		const uint32_t totalSize = 1024 * 1024 * 10;
		uint32_t bufferOffset;
	};
	static DebugData s_DebugData;
};