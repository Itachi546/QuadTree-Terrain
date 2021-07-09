#pragma once

#include <string>
#include <algorithm>
class Resource
{
public:
	Resource(const std::string& name) : m_name(name)
	{
		std::transform(name.begin(), name.end(), name.begin(), std::tolower);
	}

	std::string get_name() { return m_name; }
private:
	std::string m_name;
};