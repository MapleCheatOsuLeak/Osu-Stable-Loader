#pragma once

#include <vector>

struct ImageSection
{
	int Address;
	std::vector<unsigned char> Data;
	int Protection;
	int ProtectionSize;
};