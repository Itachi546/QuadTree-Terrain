#pragma once

#include "plane.h"
#include <array>

class Frustum
{
public:
	Frustum(){}
	

	// points order are LTBa, RTBa, RBBa, LBBa, LTF, RTF, RBF, LBF
	void set_points(const std::array<glm::vec3, 8>& points)
	{
		m_points = points;
		create_planes();
	}

	bool intersect_box(const BoundingBox& box)
	{
		glm::vec3 min = box.min;
		glm::vec3 max = box.max;

		for (int i = 0; i < 6; ++i)
		{
			glm::vec3 p = min;
			glm::vec3 n = max;

			glm::vec3 normal = m_planes[i].normal;
			if (normal.x >= 0.0f)
			{
				p.x = max.x;
				n.x = min.x;
			}
			if (normal.y >= 0.0f)
			{
				p.y = max.y;
				n.y = min.y;
			}
			if (normal.z >= 0.0f)
			{
				p.z = max.z;
				n.z = min.z;
			}

			if (m_planes[i].get_distance_to(p) < 0.0f)
				return false;
			else if (m_planes[i].get_distance_to(n) < 0.0f)
				return true;
			//if (outside)
				//return false;
		}
		return true;
	}

	const std::array<glm::vec3, 8>& get_points() const
	{
		return m_points;
	}

private:
	std::array<glm::vec3, 8> m_points = {};
	std::array<Plane, 6> m_planes = {};

	// All plane must have normal facing inside the frustum
    // Left, Right, Top, Bottom, Near, Far
	void create_planes()
	{
		// Left
		m_planes[0].set(m_points[3], m_points[7], m_points[0]);
		// Right
		m_planes[1].set(m_points[1], m_points[6], m_points[2]);

		//Top
		m_planes[2].set(m_points[0], m_points[4], m_points[1]);
		// Bottom
		m_planes[3].set(m_points[3], m_points[2], m_points[7]);

		// Near
		m_planes[4].set(m_points[3], m_points[0], m_points[2]);
		//Far
		m_planes[5].set(m_points[7], m_points[6], m_points[4]);
	}
};