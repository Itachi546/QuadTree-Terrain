#pragma once

#include "core/math.h"
#include "gpu_memory.h"

#include <vector>
#include <memory>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};
class Context;

class Mesh
{
	friend class Scene;
public:
	Mesh() {}
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const BoundingBox& boundingBox) : 
		vertices(vertices),
		indices(indices),
		indices_count(static_cast<uint32_t>(indices.size())),
		boundingBox(boundingBox){}

	void calculate_bounding_box();
	void finalize(Context* context);

	VertexBufferView* get_vb() { return vb; }
	IndexBufferView* get_ib() { return ib; }

	BoundingBox get_bounding_box() { return boundingBox; }
	void set_bounding_box(BoundingBox box) { box = boundingBox; }
	uint32_t get_indices_count() { return indices_count; }

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	uint32_t indices_count;
	BoundingBox boundingBox;
private:
	VertexBufferView* vb = nullptr;
	IndexBufferView* ib = nullptr;


};
