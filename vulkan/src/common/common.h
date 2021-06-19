#pragma once

#include "core/base.h"
#include <fstream>

inline std::string load_file(const char* filename)
{
	std::ifstream inFile(filename, std::ios::binary);
	if (inFile)
		return std::string(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>());
	else
		ASSERT_MSG(0, "File doesn't exist");
	return std::string{};
}

